#include "form_data_table_widget.h"

form_data_table_widget::form_data_table_widget(QWidget *parent)
: QTableWidget(parent)
{
}

form_data_table_widget::~form_data_table_widget()
{

}

void form_data_table_widget::set_default_column_size()
{
	setColumnWidth(FORM_DATA_COLUMN_NAME, 100);
	setColumnWidth(FORM_DATA_COLUMN_CONTENT_TYPE, 100);
	setColumnWidth(FORM_DATA_COLUMN_VALUE, 600);
}

void form_data_table_widget::showEvent(QShowEvent *event)
{
	set_default_column_size();
}

void form_data_table_widget::set_data(const http_form_data_list& datas)
{
	setRowCount(datas.size());
	for (int i = 0; i < datas.size(); i++){
		http_form_data_ptr data_ptr = datas[i];

		setItem(i, FORM_DATA_COLUMN_NAME, new QTableWidgetItem(data_ptr->name()));
		setItem(i, FORM_DATA_COLUMN_CONTENT_TYPE, new QTableWidgetItem(data_ptr->content_type()));
		setItem(i, FORM_DATA_COLUMN_VALUE, new QTableWidgetItem(data_ptr->value()));
	}

}

http_form_data_list form_data_table_widget::data()
{
	http_form_data_list datas;
	for (int row = 0; row < rowCount(); row++){
		http_form_data_ptr data_ptr(new http_form_data);
		QTableWidgetItem *name_item = item(row, FORM_DATA_COLUMN_NAME);
		QString name = name_item ? name_item->data(0).toString() : "";
		data_ptr->set_name(name);

		QTableWidgetItem *content_type_item = item(row, FORM_DATA_COLUMN_CONTENT_TYPE);
		QString content_type = content_type_item ? content_type_item->data(0).toString() : "Body";
		data_ptr->set_content_type(content_type);

		QTableWidgetItem *value_item = item(row, FORM_DATA_COLUMN_VALUE);
		QString value = value_item ? value_item->data(0).toString() : "";
		data_ptr->set_value(value);

		datas.append(data_ptr);
	}

	return datas;
}
