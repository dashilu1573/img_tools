#include <iostream>
#include <fstream>
//#include <opencv2/core.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

/********************************************************************
*函数名：main
*参  数：
*功  能：函数入口
*备  注：
********************************************************************/

static char Image_code_binary_to_hex(int n)
{
    char ch = ' ';
    switch (n) {
        case 0:
            ch = '0';
            break;
        case 1:
            ch = '1';
            break;
        case 2:
            ch = '2';
            break;
        case 3:
            ch = '3';
            break;
        case 4:
            ch = '4';
            break;
        case 5:
            ch = '5';
            break;
        case 6:
            ch = '6';
            break;
        case 7:
            ch = '7';
            break;
        case 8:
            ch = '8';
            break;
        case 9:
            ch = '9';
            break;
        case 10:
            ch = 'a';
            break;
        case 11:
            ch = 'b';
            break;
        case 12:
            ch = 'c';
            break;
        case 13:
            ch = 'd';
            break;
        case 14:
            ch = 'e';
            break;
        case 15:
            ch = 'f';
            break;
        default:
            break;
    }
    return ch;
    
}
/*******************************按符号分割字符*************************************/
static void splitChar(string s,vector<string>& split_word, string seg)
{
    size_t last = 0;
    size_t index=s.find_first_of(seg,last);
    while (index!=std::string::npos)
    {
        split_word.push_back(s.substr(last,index-last));
        last=index+1;
        index=s.find_first_of(seg,last);
    }
    if (index-last>0)
    {
        split_word.push_back(s.substr(last,index-last));
    }
}

/*
 * 功能：利用 phash 法计算图像指纹
 * 输入：image cv::Mat 输入图像
 * 输出：hash string 计算得到的指纹
 */
string ImagePHash(cv::Mat image)
{
    string hash = ""; // 用于保存hash值
    cv::Mat imageGray; // 转换后的灰度图像
    cv::Mat imageFinger; // 缩放后的8x8的指纹图像
    int fingerSize = 8; // 指纹图像的大小
    int dctSize = 32; // dct变换的尺寸大小
    vector<int> comps;
    
    if (3 == image.channels()) // rgb -> gray
    {
        cv::cvtColor(image, imageGray, CV_RGB2GRAY);
    }
    else
    {
        imageGray = image.clone();
    }
    
    cv::resize(imageGray, imageFinger, cv::Size(dctSize, dctSize)); // 图像缩放
    imageFinger.convertTo(imageFinger, CV_32F); // 转换为浮点型
    cv::dct(imageFinger, imageFinger); // 对缩放后的图像进行dct变换
    imageFinger = imageFinger(cv::Rect(0, 0, fingerSize, fingerSize)); // 取低频区域
    
    /* 对dct变换后的系数取对数 */
    for (int i = 0; i < fingerSize; i++)
    {
        float* data = imageFinger.ptr<float>(i);
        for (int j = 0; j < fingerSize; j++)
        {
            data[j] = logf(abs(data[j]));
        }
    }
    
    cv::Scalar imageMean = cv::mean(imageFinger); // 求均值
    
    /* 计算图像哈希指纹，小于等于均值为0，大于为1 */
    for (int i = 0; i < fingerSize; i++)
    {
        float* data = imageFinger.ptr<float>(i);
        for (int j = 0; j < fingerSize; j++)
        {
            if (data[j] > imageMean[0])
            {
                comps.push_back(1);
            }
            else
            {
                comps.push_back(0);
            }
        }
    }
    
    for (int i = 0; i < comps.size(); i += 4) {
        hash += Image_code_binary_to_hex((comps[i] << 3) + (comps[i + 1] << 2)
                                         + (comps[i + 2] << 1) + comps[i + 3]);
    }
    
    comps.clear();
    
    return hash;
}

/*
 * 功能：利用 dhash 法计算图像指纹
 * 输入：image cv::Mat 输入图像
 * 输出：hash uint64_t 计算得到的指纹
 */

string ImageDHash(cv::Mat image)
{
    string hash = ""; // 用于保存hash值
    cv::Mat imageGray; // 转换后的灰度图像
    cv::Mat imageFinger; // 缩放后的8x8的指纹图像
    int fingerSize = 8; // 指纹图像的大小
    vector<int> comps;
    
    if (3 == image.channels()) // rgb -> gray
    {
        cv::cvtColor(image, imageGray, CV_RGB2GRAY);
    }
    else
    {
        imageGray = image.clone();
    }
    
    cv::resize(imageGray, imageFinger, cv::Size(fingerSize+1, fingerSize)); // 图像缩放
    
    /* 计算图像哈希指纹，对于前八列的点，如果某个点大于它右侧的点，则为1，否则为0 */
    for (int i = 0; i < fingerSize; i++)
    {
        float* data = imageFinger.ptr<float>(i);
        for (int j = 0; j < fingerSize; j++)
        {
            if (data[j] > data[j+1])
            {
                comps.push_back(1);
            }
            else
            {
                comps.push_back(0);
            }
        }
    }
    
    for (int i = 0; i < comps.size(); i += 4) {
        hash += Image_code_binary_to_hex((comps[i] << 3) + (comps[i + 1] << 2)
                                        + (comps[i + 2] << 1) + comps[i + 3]);
    }
    comps.clear();
    
    return hash;
}


int main(int argc, char *argv[])
{
    char   *url_file       = NULL;
    FILE   *img_code       = NULL;
    Mat    img;
    string hash = "";
    
    //判断参数
    if(argc < 3)
    {
        cout << "Unknown Option"        << endl;
        cout << "argv[1]:图片信息(item_id, updated_time, url (tab分割))"        << endl;
        cout << "argv[2]:保存文件名"      << endl;
        return -1;
    }
    url_file   = argv[1];
    img_code = fopen(argv[2], "w");
    
    string line;
    std::ifstream img_paths(url_file);
    while (std::getline(img_paths, line))
    {
        hash = "";
        //cout << img_path << endl
        vector<string>  split_char;
        splitChar(line, split_char, "\t");
        img = imread(split_char[2], CV_LOAD_IMAGE_GRAYSCALE);
        
        if(img.data == NULL)
        {
            continue;
        }
        
        hash += ImageDHash(img);
        hash += ImagePHash(img);
        
        fprintf(img_code, "%s\t", split_char[0].c_str());
        fprintf(img_code, "%s\t", split_char[1].c_str());
        fprintf(img_code, "%s\n", hash.c_str());
    }
    
    img_paths.close();
    fclose(img_code);
    
    return 0;
}


