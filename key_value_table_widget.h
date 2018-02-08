#ifndef KEY_VALUE_TABLE_WIDGET_H
#define KEY_VALUE_TABLE_WIDGET_H

#include "table_widget.h"
#include "http_data.h"

class key_value_table_widget : public table_widget
{
	Q_OBJECT

public:
    enum key_value_column_index_t{
        KEY_VALUE_COLUMN_KEY = 0,
        KEY_VALUE_COLUMN_USED = 1,
        KEY_VALUE_COLUMN_VALUE = 2,
        KEY_VALUE_COLUMN_COUNT = 3
	};

    key_value_table_widget(QWidget *parent);
    ~key_value_table_widget();
	void set_default_column_size();
    void set_post_type(http_data::post_type_t post_type);
	void set_data(const http_data_list& v);
	http_data_list data();
protected:
	void showEvent(QShowEvent *event);
public:
    void insert();
public slots:
    void cellEntered(int row, int column);
    void itemEntered(QTableWidgetItem *item);
private:
    http_data::post_type_t _post_type;
	
};

#endif // KEY_VALUE_TABLE_WIDGET_H
