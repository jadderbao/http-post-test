#include "key_value_table_widget.h"
#include "http_data.h"

key_value_table_widget::key_value_table_widget(QWidget *parent)
	: table_widget(parent)
    , _post_type()
{
    connect(this, &QTableWidget::cellEntered, this, &key_value_table_widget::cellEntered);
    connect(this, &QTableWidget::itemEntered, this, &key_value_table_widget::itemEntered);
}

key_value_table_widget::~key_value_table_widget()
{

}

void key_value_table_widget::set_default_column_size()
{
    setColumnWidth(KEY_VALUE_COLUMN_KEY, 100);
    setColumnWidth(KEY_VALUE_COLUMN_USED, 60);
    setColumnWidth(KEY_VALUE_COLUMN_VALUE, 300);
}

void key_value_table_widget::set_post_type(http_data::post_type_t post_type)
{
    _post_type = post_type;
}

void key_value_table_widget::showEvent(QShowEvent *event)
{
    set_default_column_size();
}

void key_value_table_widget::insert()
{
    table_widget::insert();    
    int row = currentRow();
    if(row < 0){
        return;
    }

    setItem(row, KEY_VALUE_COLUMN_USED, new QTableWidgetItem("true"));
}

void key_value_table_widget::cellEntered(int row, int column)
{
    if(row != rowCount() -1 ){
        return;
    }

    QTableWidgetItem *key_item = item(row, KEY_VALUE_COLUMN_KEY);
    QTableWidgetItem *value_item = item(row, KEY_VALUE_COLUMN_VALUE);
    if(( key_item && !key_item->data(0).toString().isEmpty())
            || (value_item && !value_item->data(0).toString().isEmpty()) ){

        insert();
    }

}

void key_value_table_widget::itemEntered(QTableWidgetItem *item)
{
    if(item->row() != rowCount() -1 ){
        return;
    }

}

void key_value_table_widget::set_data(const http_data_list& datas)
{	
	setRowCount(datas.size());
	for (int i = 0; i < datas.size(); i++){
		http_data_ptr data_ptr = datas[i];

		setItem(i, KEY_VALUE_COLUMN_KEY, new QTableWidgetItem(data_ptr->key()));
        setItem(i, KEY_VALUE_COLUMN_USED, new QTableWidgetItem(data_ptr->used() ? "true" : "false"));
		setItem(i, KEY_VALUE_COLUMN_VALUE, new QTableWidgetItem(data_ptr->value()));
	}
}

http_data_list key_value_table_widget::data()
{
	http_data_list datas;
	for (int row = 0; row < rowCount(); row++){
		http_data_ptr data_ptr(new http_data);
		QTableWidgetItem *key_item = item(row, KEY_VALUE_COLUMN_KEY);
		QString key = key_item ? key_item->data(0).toString() : "";
		data_ptr->set_key(key);

        QTableWidgetItem *used_item = item(row, KEY_VALUE_COLUMN_USED);
        bool used = used_item ? used_item->data(0).toBool() : false;
        data_ptr->set_used(used);

		QTableWidgetItem *value_item = item(row, KEY_VALUE_COLUMN_VALUE);
		QString value = value_item ? value_item->data(0).toString() : "";
		data_ptr->set_value(value);

        data_ptr->set_post_type(_post_type);

		datas.append(data_ptr);
	}

	return datas;
}
