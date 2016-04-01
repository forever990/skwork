#include "algo_com.h"
#include "a20_gpio.h"
#include "a20_settime.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <sys/time.h>
#include "opencv2/opencv.hpp"

#include "picture_process.h"

#ifdef ANDROID
#define PIC_PATH "/mnt/asec/"
#define STORAGE_PATH "/mnt/extsd0/"
#else
#define PIC_PATH
#define STORAGE_PATH
#endif

void gps_callback(GpsLocationData *location)
{
	if (!location)
		return;

	printf("gps: %d %f %f %f %f %f %f %lx\n", location->flags,
			location->latitude, location->longitude,
			location->altitude,
			location->speed, location->bearing,
			location->accuracy, location->timestamp);

}

int savefile(const char *filename, char *buf, int size)
{
	int fd = open(filename, O_RDWR|O_CREAT|O_TRUNC, 0666);

	if (fd < 0)
		return -1;

	int ret = write(fd, buf, size);

	if (ret != size)
		return -2;

	close(fd);

	return ret;
}


/*
 * save the file to /mnt/asec
 * /mnt/asec is a ramfs, which is fast and reliable
 */
int save_vio_pics(struct vio_pic *pics)
{
	int i;
	int ret;
	for (i=0; i<3; i++) {
		char filepath[64];
		snprintf(filepath, sizeof(filepath), PIC_PATH"%d.jpg", i);
		ret = savefile(filepath, pics[i].buf, pics[i].size);
		if (ret < 0) {
			printf("save file err\n");
			return ret;
		}
	}

	return 0;
}

void vio_callback(struct violation_pic_info *infos, struct vio_pic *pics)
{
    skcl::PicHelper pic_helper;
    cv::Mat imgs[3];
    cv::Mat result_img;

	save_vio_pics(pics);

    imgs[0] = cv::imread(PIC_PATH"0.jpg", CV_LOAD_IMAGE_UNCHANGED);
    imgs[1] = cv::imread(PIC_PATH"1.jpg", CV_LOAD_IMAGE_UNCHANGED);
    imgs[2] = cv::imread(PIC_PATH"2.jpg", CV_LOAD_IMAGE_UNCHANGED);

	int index = 2;
	int area = infos[2].width * infos[2].height;
	if(infos[0].width * infos[0].height > area){
		index = 0;
		area = infos[0].width * infos[0].height;
	}
	if(infos[1].width * infos[1].height > area)
		index = 1;

	cv::Rect r(infos[index].left, infos[index].top, infos[index].width, infos[index].height);
    cv::Mat zoom_img;
    if (!pic_helper.ZoomPic(imgs[index], r, &zoom_img)) {
		printf("zoom error\n");
        return;
	}
    pic_helper.MergePic(imgs, zoom_img, &result_img);

    pic_helper.AddPicOSD(result_img, L"汉字wchar", 10, 200, 40, CV_RGB(255, 0, 0));
    pic_helper.AddPicOSD(result_img, "汉字！char", 10, 300, 50, CV_RGB(255, 0, 0));

    time_t cur_time;
    time(&cur_time);

	static int i = 0;
	char filepath[64];
    snprintf(filepath, sizeof(filepath), STORAGE_PATH"result%d_%ld.jpg", i, cur_time);
	i++;
	printf("save %s\n", filepath);

	cv::imwrite(filepath, result_img);

}

void image_callback(char *buf) {
#ifndef ANDROID
    cv::Mat temp_mat = cv::Mat(cv::Size(1280, 720), CV_8UC3, buf);
    cv::imshow("image", temp_mat);
    cv::waitKey(1);
#endif
}

//#define TEST_SETTIME
//#define TEST_GPIO

int main()
{
	init_algo_com(gps_callback, vio_callback, image_callback);
#ifdef TEST_SETTIME
	GpsLocationData gps;
	gps.latitude = 22.538158;
	gps.longitude = 113.947722;
	gps.altitude = 30.100000;
	gps.speed = 0;
	gps.bearing = 250.990005;
	gps.accuracy = 3.700000;
	gps.timestamp = 1452822799000;
	a20_set_time(&gps);
#endif

	int i = 0;
	int ret;
	while(1) {
#ifdef TEST_GPIO
		i = !i;
		ret = a20_set_gpio_output(0, i);
		printf("0 ret %d\n", ret);
		ret = a20_set_gpio_output(1, i);
		printf("1 ret %d\n", ret);
		ret = a20_set_gpio_output(2, i);
		printf("2 ret %d\n", ret);
		ret = a20_set_gpio_output(3, i);
		printf("3 ret %d\n", ret);
		ret = a20_set_gpio_output(4, i);
		printf("4 ret %d\n", ret);
		ret = a20_set_gpio_output(5, i);
		printf("5 ret %d\n", ret);
		ret = a20_set_gpio_output(6, i);
		printf("6 ret %d\n", ret);
#endif
		sleep(1);
	}
}
