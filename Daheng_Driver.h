#include <iostream>
#include <opencv2/opencv.hpp>
#include "yaml-cpp/yaml.h"
#include "sys/stat.h"
#include <DxImageProc.h>
#include <GxIAPI.h>
using namespace std;


namespace Daheng_camera
{
    class DHCamera {
    private:
        // 存储文件状态信息
        struct stat             result;
        GX_STATUS               status = GX_STATUS_SUCCESS;
        GX_DEV_HANDLE           hDevice = NULL;
        GX_OPEN_PARAM           stOpenParam;
        PGX_FRAME_BUFFER        pFrameBuffer;
        GX_FRAME_DATA           stFrameData;
        uint32_t                nDeviceNum = 0;
        std::string             init_config_path = "../init_config.yaml";
        std::map<std::string, double> dictionary;
    public:
        // 初始化参数
        bool InitConfig();
        bool InitCamera();
        bool SetParam();
        bool GetFileChangeTime(std::time_t &time);
        bool DynamicParameter(std::time_t &initialTime);
        bool ReConnect();
        bool GetMat(cv::Mat &image);
        ~DHCamera();

    };
}

