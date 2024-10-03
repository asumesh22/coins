#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;

Mat hough(char* filename) {
    
    // Loads an image
    Mat src = imread( samples::findFile( filename ), IMREAD_COLOR );

    // Gaussian blur the image
    int num_blurs = 20;

    for (int i = 0; i < num_blurs; i++) {
        GaussianBlur(src, src, Size(3, 3), 0);
    }

    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    medianBlur(gray, gray, 5);
    vector<Vec3f> circles;

    HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
                 gray.cols/8,  // change this value to detect circles with different distances to each other
                 100, 30, 75, 150 // change the last two parameters
            // (min_radius & max_radius) to detect larger circles
    );

    for( size_t i = 0; i < circles.size(); i++ ) {
        Vec3i c = circles[i];
        Point center = Point(c[0], c[1]);
        // circle center
        circle( src, center, 1, Scalar(0,255,0), 3, LINE_AA);

        // circle outline
        int radius = c[2];
        circle( src, center, radius, Scalar(255,0,255), 3, LINE_AA);
    }

    // imshow("detected circles", src);
    return src;
}

vector<Vec3f> hough2(char* filename) {
    
    // Loads an image
    Mat src = imread( samples::findFile( filename ), IMREAD_COLOR );

    // Gaussian blur the image
    int num_blurs = 20;

    for (int i = 0; i < num_blurs; i++) {
        GaussianBlur(src, src, Size(3, 3), 0);
    }

    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);
    medianBlur(gray, gray, 5);
    vector<Vec3f> circles;

    HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
                 gray.cols/8,  // change this value to detect circles with different distances to each other
                 100, 30, 75, 150 // change the last two parameters
            // (min_radius & max_radius) to detect larger circles
    );

    return circles;
}

Mat coinsTestCase(Mat src) {
    Mat img(src.rows, src.cols, CV_8UC3, Scalar(255, 255, 255));
    
    int col = img.cols;
    int row = img.rows;

    // Grids
    
    int gridWidth = (int)(col/4);
    int gridHeight = (int)(row/2);

    int grid1startR = 0;
    int grid1startC = 0;
    int grid2startR = 0;
    int grid2startC = gridWidth;

    // swap grid1 and grid2
    for (int i = 0; i < row; i ++) {
        for (int j = 0; j < col; j ++) {
            // pt -> row, col
            // if pt in grid 1
            if (grid1startC <= j && grid1startC+gridWidth >= j && grid1startR <= i && grid1startR+gridHeight >= i) {
                img.at<Vec3b>(i, j) = src.at<Vec3b>(i, j+gridWidth);
            } else if (grid2startC <= j && grid2startC+gridWidth >= j && grid2startR <= i && grid2startR+gridHeight >= i) { // if pt in grid 2
                img.at<Vec3b>(i, j) = src.at<Vec3b>(i, j-gridWidth);
            } else {
                img.at<Vec3b>(i, j) = src.at<Vec3b>(i, j);
            }

            
        }
    }

    return img;
}

int calculateRadius(Mat detected, int row, int col) {
    int radius = 0;
    
    int r1 = 0;
    int r2 = 0;
    int r3 = 0;
    int r4 = 0;

    Vec3b pink = {255, 0, 255};


    // left
    while (detected.at<Vec3b>(row, col+r1) != pink) {
        r1++;
    }

    // right
    while (detected.at<Vec3b>(row, col-r2) != pink) {
        r2++;
    }

    // up
    while (detected.at<Vec3b>(row+r3, col) != pink) {
        r3++;
    }

    // down
    while (detected.at<Vec3b>(row-r4, col) != pink) {
        r4++;
    }

    radius = (int) (r1+r2+r3+r4)/4;

    return radius;
}

int main(int argc, char** argv) {
    Mat src = imread("coins.jpg", IMREAD_COLOR);
    Mat coinsTC = coinsTestCase(src);
    imwrite("coinsTestCase.png", coinsTC);

    // parse the initial file

    Mat detected = hough("coins.jpg");
    int coin_centers[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int i = 0;

    // coin centers assuming they are in a grid
    for (int c = detected.cols/8; c < detected.cols; c+=detected.cols/4) {
        for (int r = detected.rows/4; r < detected.rows; r+=detected.rows/2) {
            coin_centers[i] = r * detected.cols + c;
            i++;
        }
    }

    // calculate coin radii
    int penny_up = 0;
    int penny_down = 0;
    int nickel_up = 0;
    int nickel_down = 0;
    int dime_up = 0;
    int dime_down = 0;
    int quarter_up = 0;
    int quarter_down = 0;

    // first center is penny up
    penny_up = calculateRadius(detected, coin_centers[0]/detected.cols, coin_centers[0]%detected.cols);
    penny_down = calculateRadius(detected, coin_centers[1]/detected.cols, coin_centers[1]%detected.cols);

    nickel_up = calculateRadius(detected, coin_centers[2]/detected.cols, coin_centers[2]%detected.cols);
    nickel_down = calculateRadius(detected, coin_centers[3]/detected.cols, coin_centers[3]%detected.cols);

    dime_up = calculateRadius(detected, coin_centers[4]/detected.cols, coin_centers[4]%detected.cols);
    dime_down = calculateRadius(detected, coin_centers[5]/detected.cols, coin_centers[5]%detected.cols);

    quarter_up = calculateRadius(detected, coin_centers[6]/detected.cols, coin_centers[6]%detected.cols);
    quarter_down = calculateRadius(detected, coin_centers[7]/detected.cols, coin_centers[7]%detected.cols);

    cout << penny_up << " " << penny_down << endl;
    cout << nickel_up << " " << nickel_down << endl;
    cout << dime_up << " " << dime_down << endl;
    cout << quarter_up << " " << quarter_down << endl;

    int penny = (penny_up+penny_down)/2;
    int nickel = (nickel_up+nickel_down)/2;
    int dime = (dime_up+dime_down)/2;
    int quarter = (quarter_up+quarter_down)/2;

    // end of parse file

    char* filename = "test_cases/case0000.png";

    Mat img = imread(filename, IMREAD_COLOR);
    vector<Vec3f> circles = hough2(filename);

    imshow("detected", detected);
    waitKey();

    int detectedCircles = circles.size();

    int totalSum = 0;
    for (int j = 0; j < detectedCircles; j++) {
        Vec3i cir = circles[j];

        int cirRad = cir[2];

        int minDist = 1000;
        int value = 0;

        if (abs(cirRad-penny) < minDist) {
            minDist = abs(cirRad-penny);
            value = 1;
        }

        if (abs(cirRad-nickel) < minDist) {
            minDist = abs(cirRad-nickel);
            value = 5;
        }

        if (abs(cirRad-dime) < minDist) {
            minDist = abs(cirRad-dime);
            value = 10;
        }

        if (abs(cirRad-quarter) < minDist) {
            minDist = abs(cirRad-quarter);
            value = 25;
        }

        totalSum += value;
    }

    cout << totalSum << endl;

    return -1;
}