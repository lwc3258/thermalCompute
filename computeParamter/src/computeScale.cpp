#include "computeScale.hpp"
#include <memory>

//thermal and visible image size
const int thermalWidth = 640;
const int thermalHeight = 512;
const int thermalFrameSize = thermalWidth * thermalHeight * 3 / 2;

const int visibleWidth = 1920;
const int visibleHeight = 1080;
const int visibleFrameSize = visibleWidth * visibleHeight * 3 / 2;

void computeScaleFactor()
{
    std::vector<float> vScale;
    for(size_t i = 0;i < (points1.size() ); ++i)
    {
        for(size_t j = 0;j < (points1[i].size() - 1); ++j)
        {
            float thermalDiff = points1[i ][j + 1].x - points1[i][j].x;
            float visibleDiff = points2[i][j + 1].x - points2[i][j].x;
            float TVScale = thermalDiff / visibleDiff;
            vScale.push_back(TVScale);
        }
    }

    //compute average scale
    float sum = 0;
    for(auto i : vScale)
    {
        sum += i;
    }

    float averageScale = sum / vScale.size();
    finalScale = averageScale - 0.01;
    std::cout << "Rusult of average scale :" << finalScale << std::endl;
}

bool run(cv::Mat thermalImage,cv::Mat visibleImage)
{
    img1 = thermalImage.clone();
    img2 = visibleImage.clone();

//    imshow("thermal",img1);
//    imshow("visible",img2);
//    waitKey(0);

    //invert the image color
    //invertMatIntensities(img2,img2);

    boardSize.height = boardHeight;
    boardSize.width = boardWidth;
    //for video or image data set,first need to use this function to save all the points,then for later process.
    loadImagePoints();

    //To compute the scale factor
    if(points1.size() > 0 && points2.size() > 0)
    {
        computeScaleFactor();
        return true;
    }
    else
    {
        std::cout << "Compute scale factor failed !!!" << std::endl;
        return false;
    }
}

void invertMatIntensities(const cv::Mat& src, cv::Mat& dst)
{
    if ((dst.size() != src.size()) && (dst.type() != src.type())) {
        dst.release();
        dst = cv::Mat(src.size(), src.type());
    }

    if (src.type() == CV_8UC1)
    {
        for (int iii = 0; iii < src.rows; iii++)
        {
            for (int jjj = 0; jjj < src.cols; jjj++)
            {
                if(dst.at<unsigned char>(iii,jjj) >70)
                    dst.at<unsigned char>(iii,jjj) =255;
                else
                    dst.at<unsigned char>(iii,jjj) =0;
            }
        }
    }
    else if (src.type() == CV_8UC3)
    {
        for (int iii = 0; iii < src.rows; iii++)
        {
            for (int jjj = 0; jjj < src.cols; jjj++)
            {
                dst.at<cv::Vec3b>(iii, jjj)[0] = 255 - src.at<cv::Vec3b>(iii, jjj)[0];
                dst.at<cv::Vec3b>(iii, jjj)[1] = 255 - src.at<cv::Vec3b>(iii, jjj)[1];
                dst.at<cv::Vec3b>(iii, jjj)[2] = 255 - src.at<cv::Vec3b>(iii, jjj)[2];
            }
        }
    }
}

void loadImagePoints() {

    thermalSize = img1.size();
    visibleSize = img2.size();

    cvtColor(img1, gray1, CV_RGB2GRAY);
    cvtColor(img2, gray2, CV_RGB2GRAY);

    bool found1 = false, found2 = false;

    found1 = findCirclesGrid( gray1, boardSize, corners1, CALIB_CB_ASYMMETRIC_GRID );
    found2 = findCirclesGrid( gray2, boardSize, corners2, CALIB_CB_ASYMMETRIC_GRID );

    std::cout << "Find corner1's size:" << corners1.size() << std::endl;
    std::cout << "Find corner2's size:" << corners2.size() << std::endl;

    //find the stereo pair image's points,save them in vector,later for compute camera calibration and scaleFactor
    if (found1 && found2) {
        cout << ". Found corners!" << endl;
        for(auto &i : corners1)
        {
            std::cout << "Points1 data check:" << i.x << " " << i.y << std::endl;
        }

        for(auto &i : corners2)
        {
            std::cout << "Points2 data check:" << i.x << " " << i.y << std::endl;
        }
        points1.push_back(corners1);
        points2.push_back(corners2);
    }
}

void runCalibrationAndSave()
{
    std::vector<cv::Mat> rvecs1,rvecs2,tvecs1,tvecs2;
    std::vector<float> reprojErrors1,reprojErrors2;

    K1 = cv::Mat::eye(3,3,CV_64F);
    K2 = cv::Mat::eye(3,3,CV_64F);

    if(flag & CV_CALIB_FIX_ASPECT_RATIO)
    {
        K1.at<double>(0,0) = 1.0;
        K2.at<double>(0,0) = 1.0;
    }

    D1 = cv::Mat::zeros(8,1,CV_64F);
    D2 = cv::Mat::zeros(8,1,CV_64F);

    std::vector<std::vector<cv::Point3f>>objectPoints1(1),objectPoints2(1);
    objectPoints1[0].clear();
    objectPoints2[0].clear();

    //calc board corner positions
    for(int i = 0;i < boardSize.height; ++i)
        for(int j = 0;j < boardSize.width; ++j)
        {
            objectPoints1[0].push_back(cv::Point3f(float((2 * j + i % 2) * squareSize),float(i * squareSize ),0));
            objectPoints2[0].push_back(cv::Point3f(float((2 * j + i % 2) * squareSize),float(i * squareSize ),0));
        }

    //Find intrinsic and extrinsic camera parameters
    double rms1 = calibrateCamera(objectPoints1,imagePoints1,thermalSize,K1,D1,rvecs1,tvecs1,flag | CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5);
    double rms2 = calibrateCamera(objectPoints2,imagePoints2,  visibleSize,K2,D2,rvecs2,tvecs2,flag | CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5);

    std::cout<<"Re-projection error reported by calibrateCamera:"<<std::endl;
    std::cout<<"thermal camera re-projection error:"<<rms1<<std::endl;
    std::cout<<"visible camera re-projection error:"<<rms2<<std::endl;

    bool ok1 =  checkRange(K1) && checkRange(D1);
    bool ok2 =  checkRange(K2) && checkRange(D2);

    double thermalTotalError = computeReprojectErrors(objectPoints1,imagePoints1,rvecs1,tvecs1,K1,D1,reprojErrors1);
    double visibleTotalError = computeReprojectErrors(objectPoints2,imagePoints2,rvecs2,tvecs2,K2,D2,reprojErrors2);

    if(ok1 && ok2)
    {
        std::cout<<"Calibration succeeded !"<<std::endl;
        std::cout<<"thermal camera calibrate total error:"<<thermalTotalError<<std::endl;
        std::cout<<"visible camera calibrate total error:"<<visibleTotalError<<std::endl;

        //Nothing done for now.
        saveCameraParams();
    }
}

double computeReprojectErrors(const vector<vector<Point3f> >& objectPoints,
                                                 const vector<vector<Point2f> >& imagePoints,
                                                 const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                                 const Mat& cameraMatrix , const Mat& distCoeffs,
                                                 vector<float>& perViewErrors)
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); ++i )
    {
        projectPoints( Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix,
                       distCoeffs, imagePoints2);

        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);
        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);

        totalErr += err*err;
        totalPoints += n;
    }

    return std::sqrt(totalErr / totalPoints);
}

void saveCameraParams()
{
    std::cout << "Nothing to do." << std::endl;
}

//call this to decode the image data,no used for test
Mat imageConvert(FILE* fileIn,const float width,const float height)
{
    Mat rgbImg;
    {
//        auto start = std::chrono::system_clock::now();
        const int framesize = width * height * 3 / 2;
        unsigned char* pYuvBuf = new unsigned char[framesize];

        size_t size = fread(pYuvBuf,framesize * sizeof(unsigned char),1,fileIn);

        Mat yuvImg;
        yuvImg.create(height * 3 / 2,width,CV_8UC1);
        memcpy(yuvImg.data,pYuvBuf,framesize * sizeof(unsigned char));


        cvtColor(yuvImg,rgbImg,CV_YUV2BGR_I420);

//        auto end = std::chrono::system_clock::now();
//        std::chrono::duration<double,std::milli> timeSpan = std::chrono::duration_cast<std::chrono::duration<double,std::milli>>(end - start);
//        std::cout<<"Cost time of image convert:"<<timeSpan.count()<<std::endl;

#if 1
        imshow("yuv",yuvImg);//only show Y channel.
        imshow("rgbImg",rgbImg);
        waitKey(0);
#endif
    }
    fclose(fileIn);
    //destroyWindow("yuv");
    return rgbImg;
}

void runComputeShift(cv::Mat thermal,cv::Mat visible)
{
    cv::Mat gray;
    cv::Mat testImage = cv::imread("./testImage",-1);
    cvtColor(testImage, gray, CV_RGB2GRAY);

    bool found;
    std::vector< cv::Point2f > corners;
    found = findChessboardCorners( gray, boardSize, corners,
        CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

    if(found)
    {
        std::cout << "found chessboard corners" << std::endl;
    }
}

int main(int argc, char** argv)
{
    //parameter num chaeck
    if(argc < 3)
    {
        std::cout<<"parameter num ERROR"<<std::endl;
        std::cout<<"Usage: ./main  ./thermalImage ./visibleImage"<<std::endl;
        return -1;
    }

    //read the yuv image data
    FILE* thermalFile = fopen("thermal.yuv","rb+");
    FILE* visibleFile = fopen("visible.yuv","rb+");

    Mat thermalImage = imageConvert(thermalFile,thermalWidth,thermalHeight);
    Mat visibleImage = imageConvert(visibleFile,visibleWidth,visibleHeight);

     if(run(thermalImage,visibleImage))
     {
         std::cout << "Compute scale factor success !" << std::endl;
     }
    return 0;
}
