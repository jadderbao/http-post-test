#include "key_value_table_widget.h"
#include "http_data.h"
#include "table_widget_delegate.h"

key_value_table_widget::key_value_table_widget(QWidget *parent)
	: table_widget(parent)
    , _post_type()
{

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

void key_value_table_widget::setItemDelegate(QAbstractItemDelegate *delegate)
{
    table_widget_delegate *widget_delgate = qobject_cast<table_widget_delegate *>(delegate);
    if(widget_delgate){
        connect(widget_delgate, &table_widget_delegate::item_editor, this, &key_value_table_widget::item_editor);
    }

    table_widget::setItemDelegate(delegate);
}


void key_value_table_widget::item_editor(const QModelIndex &index)
{
    if(index.row() != rowCount() -1 ){
        return;
    }

    int row = index.row();
    QTableWidgetItem *used_item = item(row, KEY_VALUE_COLUMN_USED);
    bool used = used_item ? used_item->data(0).toBool() : false;
    if(!used && index.column() != KEY_VALUE_COLUMN_USED){
       setItem(row, KEY_VALUE_COLUMN_USED, new QTableWidgetItem("true"));;
    }


    QTableWidgetItem *key_item = item(row, KEY_VALUE_COLUMN_KEY);
    QTableWidgetItem *value_item = item(row, KEY_VALUE_COLUMN_VALUE);
    if(( key_item && !key_item->data(0).toString().isEmpty())
        || ( value_item && !value_item->data(0).toString().isEmpty()) ){

        row = rowCount();
        setRowCount(row + 1);
        setItem(row, KEY_VALUE_COLUMN_USED, new QTableWidgetItem("false"));;
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
