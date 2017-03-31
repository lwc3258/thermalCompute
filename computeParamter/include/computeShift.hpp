#ifndef  COMPUTESCALE_HPP
#define COMPUTESCALE_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <iostream>
#include <omp.h>
#include <chrono>

using namespace std;
using namespace cv;

//The camera calibration board parameter
const int boardHeight = 11;
const int boardWidth = 3;

const float squareSize = 0.19;//square size mm.

//the main function of camera calibration and compute scale  factor.
void run(cv::Mat thermalImage,cv::Mat visibleImage);

//input image must be RGB images
void loadImagePoints() ;

//invert the image color
void invertMatIntensities(const cv::Mat& src, cv::Mat& dst);

void runCalibrationAndSave();

double computeReprojectErrors(const vector<vector<Point3f> >& objectPoints,
                              const vector<vector<Point2f> >& imagePoints,
                              const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                              const Mat& cameraMatrix , const Mat& distCoeffs,
                              vector<float>& perViewErrors);

//TODO,need to figure out the parameters.
void saveCameraParams();

void computeScaleFactor();

std::vector< std::vector< cv::Point2f > > imagePoints1, imagePoints2;

std::vector< cv::Point2f > corners1, corners2;

//compute scale points
std::vector<std::vector<cv::Point2f>> points1,points2;

cv::Mat img1, img2, gray1, gray2;

cv::Size thermalSize,visibleSize;

cv::Mat K1,K2,D1,D2;

double Fx1 = 0,Fx2 = 1;

float finalScale;

//flag to chose camera calibration parameters
int flag;

cv::Size boardSize;

#endif //CAMERACALIBRATION_HPP
