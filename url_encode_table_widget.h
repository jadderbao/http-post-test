#ifndef URL_ENCODE_TABLE_WIDGET_H
#define URL_ENCODE_TABLE_WIDGET_H

#include <QTableWidget>
#include "http_data.h"

class url_encode_table_widget : public QTableWidget
{
	Q_OBJECT

public:
	enum url_encode_column_index_t{
		URL_ENCODE_COLUMN_KEY = 0,
		URL_ENCODE_COLUMN_POST_TYPE = 1,
		URL_ENCODE_COLUMN_AUTH = 2,
		URL_ENCODE_COLUMN_VALUE_TYPE = 3,
		URL_ENCODE_COLUMN_VALUE = 4,
		URL_ENCODE_COLUMN_COUNT = 5
	};

	url_encode_table_widget(QWidget *parent);
	~url_encode_table_widget();
	void set_default_column_size();
	void set_data(const http_data_list& v);
	http_data_list data();
protected:
	void showEvent(QShowEvent *event);

private:
	
};

#endif // URL_ENCODE_TABLE_WIDGET_H
