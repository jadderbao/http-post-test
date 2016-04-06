#ifndef HEADERS_TABLE_WIDGET_H
#define HEADERS_TABLE_WIDGET_H

#include "url_encode_table_widget.h"

class headers_table_widget : public url_encode_table_widget
{
	Q_OBJECT

public:
	headers_table_widget(QWidget *parent);
	~headers_table_widget();
private:
	
};

#endif // HEADERS_TABLE_WIDGET_H
