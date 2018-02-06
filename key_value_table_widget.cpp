#include "key_value_table_widget.h"
#include "http_data.h"

key_value_table_widget::key_value_table_widget(QWidget *parent)
	: table_widget(parent)
{
}

key_value_table_widget::~key_value_table_widget()
{

}

void key_value_table_widget::set_default_column_size()
{
    setColumnWidth(URL_ENCODE_COLUMN_KEY, 100);
    setColumnWidth(URL_ENCODE_COLUMN_USED, 60);
    setColumnWidth(URL_ENCODE_COLUMN_VALUE, 300);
}

void key_value_table_widget::showEvent(QShowEvent *event)
{
	set_default_column_size();
}

void key_value_table_widget::set_data(const http_data_list& datas)
{	
	setRowCount(datas.size());
	for (int i = 0; i < datas.size(); i++){
		http_data_ptr data_ptr = datas[i];

		setItem(i, URL_ENCODE_COLUMN_KEY, new QTableWidgetItem(data_ptr->key()));
        setItem(i, URL_ENCODE_COLUMN_USED, new QTableWidgetItem(data_ptr->used()));
		setItem(i, URL_ENCODE_COLUMN_VALUE, new QTableWidgetItem(data_ptr->value()));
	}
}

http_data_list key_value_table_widget::data()
{
	http_data_list datas;
	for (int row = 0; row < rowCount(); row++){
		http_data_ptr data_ptr(new http_data);
		QTableWidgetItem *key_item = item(row, URL_ENCODE_COLUMN_KEY);
		QString key = key_item ? key_item->data(0).toString() : "";
		data_ptr->set_key(key);

        QTableWidgetItem *used_item = item(row, URL_ENCODE_COLUMN_USED);
        int used = used_item ? used_item->data(0).toInt() : 0;
        data_ptr->set_auth(used);

		QTableWidgetItem *value_item = item(row, URL_ENCODE_COLUMN_VALUE);
		QString value = value_item ? value_item->data(0).toString() : "";
		data_ptr->set_value(value);

		datas.append(data_ptr);
	}

	return datas;
}
