#include "url_encode_table_widget.h"
#include "http_data.h"

url_encode_table_widget::url_encode_table_widget(QWidget *parent)
	: table_widget(parent)
{
}

url_encode_table_widget::~url_encode_table_widget()
{

}

void url_encode_table_widget::set_default_column_size()
{
	setColumnWidth(URL_ENCODE_COLUMN_AUTH, 60);
	setColumnWidth(URL_ENCODE_COLUMN_VALUE, 300);
}

void url_encode_table_widget::showEvent(QShowEvent *event)
{
	set_default_column_size();
}

void url_encode_table_widget::set_data(const http_data_list& datas)
{	
	setRowCount(datas.size());
	for (int i = 0; i < datas.size(); i++){
		http_data_ptr data_ptr = datas[i];

		setItem(i, URL_ENCODE_COLUMN_KEY, new QTableWidgetItem(data_ptr->key()));
		setItem(i, URL_ENCODE_COLUMN_POST_TYPE, new QTableWidgetItem(data_ptr->post_type_string()));

		QTableWidgetItem *auth_item = new QTableWidgetItem(data_ptr->auth());
		auth_item->setData(Qt::DisplayRole, data_ptr->auth());
		setItem(i, URL_ENCODE_COLUMN_AUTH, auth_item);

		setItem(i, URL_ENCODE_COLUMN_VALUE_TYPE, new QTableWidgetItem(data_ptr->value_type()));
		setItem(i, URL_ENCODE_COLUMN_VALUE, new QTableWidgetItem(data_ptr->value()));
	}
}

http_data_list url_encode_table_widget::data()
{
	http_data_list datas;
	for (int row = 0; row < rowCount(); row++){
		http_data_ptr data_ptr(new http_data);
		QTableWidgetItem *key_item = item(row, URL_ENCODE_COLUMN_KEY);
		QString key = key_item ? key_item->data(0).toString() : "";
		data_ptr->set_key(key);

		QTableWidgetItem *type_item = item(row, URL_ENCODE_COLUMN_POST_TYPE);
		QString type = type_item ? type_item->data(0).toString() : "Body";
		data_ptr->set_post_type(http_data::get_post_type(type));

		QTableWidgetItem *auth_item = item(row, URL_ENCODE_COLUMN_AUTH);
		int auth = auth_item ? auth_item->data(0).toInt() : 0;
		data_ptr->set_auth(auth);

		QTableWidgetItem *value_type_item = item(row, URL_ENCODE_COLUMN_VALUE_TYPE);
		QString value_type = value_type_item ? value_type_item->data(0).toString() : "";
		data_ptr->set_value_type(value_type);

		QTableWidgetItem *value_item = item(row, URL_ENCODE_COLUMN_VALUE);
		QString value = value_item ? value_item->data(0).toString() : "";
		data_ptr->set_value(value);

		datas.append(data_ptr);
	}

	return datas;
}
