#ifndef FORM_DATA_TABLE_WIDGET_H
#define FORM_DATA_TABLE_WIDGET_H

#include <QTableWidget>
#include "http_data.h"

class form_data_table_widget : public QTableWidget
{
	Q_OBJECT

public:

	enum form_data_column_index_t{
		FORM_DATA_COLUMN_NAME = 0,
		FORM_DATA_COLUMN_CONTENT_TYPE = 1,
		FORM_DATA_COLUMN_VALUE = 2,
		FORM_DATA_COLUMN_COUNT = 3
	};

	form_data_table_widget(QWidget *parent);
	void set_default_column_size();
	~form_data_table_widget();
	void set_data(const http_form_data_list& datas);
	http_form_data_list data();
protected:
	void showEvent(QShowEvent *event);

private:
	
};

#endif // FORM_DATA_TABLE_WIDGET_H
