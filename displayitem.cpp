#include "displayitem.h"

void DisplayItem::setParameter( TCanvas *canvas, int x, int y, int width, int height, double rot )
{
	// 大きさ位置角度を設定
	this->x = x;
	this->y = y;
	
	this->width  = width;
	this->height = height;
	
	this->rot = rot;
	
	this->canvas = canvas;
}
