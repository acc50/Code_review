﻿#include "pch.h"
#include <iostream>

int main()
{
	//int copy[SIZE];
	myPoint a[30] = { {316, 353}, {253, 857}, {743, 851}, {183, 998}, {262, 439},
	{330, 732}, {418, 232}, {856, 448}, {596, 658}, {266, 100}, {890, 793}, {665, 864},
	{238, 657}, {636, 731}, {735, 606}, {562, 203}, {492, 701}, {306, 393}, {798, 939}, {921, 176},
	{637, 780}, {620, 975}, {718, 94}, {887, 749}, {756, 560}, {525, 805}, {229, 29}, {352, 570},
	{670, 589}, {644, 821} };
	/*//for (int i = 0; i < SIZE; i++)
	//	copy[i] = a[i].x;
	////SortByX(a, copy, 0, SIZE-1);  //처음 정렬은 x기준으로 정렬을 합니다.*/
	myQSort(a, 0, SIZE - 1);
	std::cout << "--------------------\n";
	for (myPoint d : a)
		std::cout << " (" << d.x << ',' << d.y << ") \n";
	std::cout << "\n--------------------\n";
	GetMinDis(a, 0, SIZE - 1);
	std::cout << "최근접거리:" << min << " 최근접쌍" << "(" << minPoint[0].x << ','
		<< minPoint[0].y << "), " << "(" << minPoint[1].x << ',' << minPoint[1].y << ")";
}