#include "displayitem.h"

void DisplayItem::setParameter( TCanvas *canvas, int x, int y, int width, int height, double rot )
{
	// �傫���ʒu�p�x��ݒ�
	this->x = x;
	this->y = y;
	
	this->width  = width;
	this->height = height;
	
	this->rot = rot;
	
	this->canvas = canvas;
}
