/*
    Date:2025.3.9
    *Description: Daheng
    *Function List:
        1.InitConfig 初始化参数
        2.InitCamera 初始化相机
        3.SetParam 设置相机参数
        4.GetFileChangeTime 获取最近一次文件修改时间
        5.DynamicParameter 动态调节相机参数
        6.ReConnect 断线重连
        7.GetMat 获取一帧图像
*/
#include "Daheng_Driver.h"

bool Daheng_camera::DHCamera::InitConfig() {
    YAML::Node config = YAML::LoadFile(init_config_path);
    dictionary["exposure"] = config["exposure"].as<double>();
    dictionary["red"] = config["balance"]["red"].as<double>();
    dictionary["green"] = config["balance"]["green"].as<double>();
    dictionary["blue"] = config["balance"]["blue"].as<double>();
    dictionary["gain"] = config["gain"].as<double>();
    return true;
}

bool Daheng_camera::DHCamera::InitCamera() {
    //起始位置调用GXInitLib()进行初始化，申请资源
    status = GXInitLib();
    if(status != GX_STATUS_SUCCESS)
    {
        cout << "init camera fail! the status is " << status << endl; 
        return false;
    }
    //枚举相机
    status = GXUpdateDeviceList(&nDeviceNum, 1000);
    if(status == GX_STATUS_SUCCESS && nDeviceNum > 0)
    {
        //打开枚举列表中的第一台设备
        stOpenParam.accessMode = GX_ACCESS_EXCLUSIVE;
        stOpenParam.openMode = GX_OPEN_INDEX;
        char index_str[] = "1";
        stOpenParam.pszContent = index_str;
        status = GXOpenDevice(&stOpenParam, &hDevice);
    }
    else 
    {
        cout << "init camera fail! the status is " << status << endl;
        return false;
    }
    return true;
}

bool Daheng_camera::DHCamera::SetParam() {
    // 曝光时间
    status = GXSetFloat(hDevice, GX_FLOAT_EXPOSURE_TIME, dictionary["exposure"]);
    // 白平衡
    status = GXSetEnum(hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_RED);
    status = GXSetFloat(hDevice, GX_FLOAT_BALANCE_RATIO, dictionary["red"]);
    status = GXSetEnum(hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_GREEN);
    status = GXSetFloat(hDevice, GX_FLOAT_BALANCE_RATIO, dictionary["green"]);
    status = GXSetEnum(hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_BLUE);
    status = GXSetFloat(hDevice, GX_FLOAT_BALANCE_RATIO, dictionary["blue"]);
    // 增益
    status = GXSetEnum(hDevice, GX_ENUM_GAIN_SELECTOR, GX_GAIN_SELECTOR_ALL);
    status = GXSetFloat(hDevice, GX_FLOAT_GAIN, dictionary["gain"]);
    if (status != GX_STATUS_SUCCESS) {
        cout << "set camera param fail! the status is " << status << endl;
        return false;
    }
    return true;
}

bool Daheng_camera::DHCamera::GetFileChangeTime(std::time_t &time) {
    if (stat(init_config_path.c_str(), &result) == 0) {
        time = result.st_mtime;
        return true;
    } else {
        std::cout << "can't get FileChangeTime" << std::endl;
        return false;
    }
}

bool Daheng_camera::DHCamera::DynamicParameter(std::time_t &initialTime) {
    std::time_t currentTime;
    GetFileChangeTime(currentTime);
    if (currentTime != initialTime) {
        cout <<  "File has been changed!" << std::endl;
        initialTime = currentTime;
        GXStreamOff(hDevice);
        InitConfig();
        SetParam();
        GXStreamOn(hDevice);
    }
    return true;
}

bool Daheng_camera::DHCamera::ReConnect() {
    GXCloseDevice(hDevice);
    status = GX_STATUS_ERROR;
    while(status != GX_STATUS_SUCCESS) {
        nDeviceNum = 0;
        while (nDeviceNum == 0) {
            GXUpdateDeviceList(&nDeviceNum, 1000);
            cout << "No camera! Try to reconnect" << std::endl;
        }
        stOpenParam.accessMode = GX_ACCESS_EXCLUSIVE;
        stOpenParam.openMode = GX_OPEN_INDEX;
        char index_str[] = "1";
        stOpenParam.pszContent = index_str;
        status = GXOpenDevice(&stOpenParam, &hDevice);
    }
    InitConfig(); 
    SetParam();  
    std::cout << "reconnect success!" << std::endl;
    return true;
}

bool Daheng_camera::DHCamera::GetMat(cv::Mat &image) {
    status = GXStreamOn(hDevice);
    if (status != GX_STATUS_SUCCESS) {
        cout << "camera pick up picture fail! the status is " << status << endl;
        return false;
    }

    status = GXDQBuf(hDevice, &pFrameBuffer, 1000);
    if (status != GX_STATUS_SUCCESS || pFrameBuffer == nullptr) {
        cout << "get the picture fail! the status is " << status << endl;
        return false;
    }

    if (pFrameBuffer->nStatus != GX_FRAME_STATUS_SUCCESS || 
        pFrameBuffer->nWidth == 0 || pFrameBuffer->nHeight == 0) {
        cout << "frame invalid! width or height is 0" << endl;
        GXQBuf(hDevice, pFrameBuffer); 
        return false;
    }

    std::vector<unsigned char> buffer(pFrameBuffer->nHeight * pFrameBuffer->nWidth * 3);
    stFrameData.pImgBuf = buffer.data();

    DxRaw8toRGB24Ex(pFrameBuffer->pImgBuf, stFrameData.pImgBuf, 
                    pFrameBuffer->nWidth, pFrameBuffer->nHeight, 
                    RAW2RGB_NEIGHBOUR, BAYERRG, false, DX_ORDER_BGR);

    cv::Mat temp_image(pFrameBuffer->nHeight, pFrameBuffer->nWidth, CV_8UC3, stFrameData.pImgBuf);
    temp_image.copyTo(image);

    cv::resize(image, image, cv::Size(640, 480));

    // 释放缓冲区
    GXQBuf(hDevice, pFrameBuffer);

    return true;
}



Daheng_camera::DHCamera::~DHCamera() {
    status = GXCloseDevice(hDevice);
    if(status != GX_STATUS_SUCCESS)
    {
        cout << "close camera fail! the status is " << status << endl;
    }
    else
    {
        //在结束的时候调用GXClosLib()释放资源
        status = GXCloseLib();
        if(status != GX_STATUS_SUCCESS)
        {
            cout << "close cameralib fail! the status is " << status << endl;
        }
        else
        {
            cout << "Camera destroyed!" << endl;
        }
    }
}

int main()
{
    Daheng_camera::DHCamera Daheng;
    Daheng.InitConfig();
    Daheng.InitCamera();
    Daheng.SetParam();
    std::time_t initaltime;
    Daheng.GetFileChangeTime(initaltime);
    while (1) {
        cv::Mat mat;
        Daheng.DynamicParameter(initaltime);
        Daheng.GetMat(mat);
        // cout << mat.size() << endl;
        if (mat.empty()) {
            Daheng.ReConnect();
        }
        else {
            cv::imshow("image", mat);
            if(cv::waitKey(5) == 27)
            {
                break;
            }
        }
    }
    return 0;
}