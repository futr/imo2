#ifndef DISPLAYITEM_H_INCLUDED
#define DISPLAYITEM_H_INCLUDED

#include <vcl.h>

enum MouseState {
	MouseStateNormal,
    MouseStateSize,
    MouseStateRotate,
    MouseStateSetting,
};

class DisplayItem;
class DisplayItemManager;

class DisplayItemManager {
public:
	void addItem( DisplayItem *item );
    void deleteItem( DisplayItem *item );
    DisplayItem *getCurrenItem();

	MouseState getMouseState( int x, int y );

	void setCanvas( TCanvas *canvas );
    void setViewport( int ltx, int lty, int rbx, int rby, double mag );

    void drawItems();

	void mouseDown( TMouseButton button, int x, int y );
    void mouseMove( int x, int y );
    void mouseUp( TMouseButton button, int x, int y );

private:
	TList *items;
};

class DisplayItem {
public:
	virtual void drawItem( int ltx, int lty, int rbx, int rby, int mx, int my, double mag ) = 0;					// èÉêàâºëz : ï`âÊ

	void setParameter( TCanvas *canvas, int x, int y, int width, int height, double rot );

private:
	TCanvas *canvas;

	int x;
	int y;
	int width;
	int height;

	double rot;
};

#endif