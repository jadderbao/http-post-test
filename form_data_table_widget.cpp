#include "form_data_table_widget.h"
#include "table_widget_delegate.h"

form_data_table_widget::form_data_table_widget(QWidget *parent)
: table_widget(parent)
{
}

form_data_table_widget::~form_data_table_widget()
{

}

void form_data_table_widget::set_default_column_size()
{
	setColumnWidth(FORM_DATA_COLUMN_NAME, 100);
    setColumnWidth(FORM_DATA_COLUMN_USED, 50);
    setColumnWidth(FORM_DATA_COLUMN_CONTENT_TYPE, 100);
	setColumnWidth(FORM_DATA_COLUMN_VALUE, 300);
}

void form_data_table_widget::showEvent(QShowEvent *event)
{
    set_default_column_size();
}

void form_data_table_widget::insert()
{
    table_widget::insert();
    int row = currentRow();
    if(row < 0){
        return;
    }

    setItem(row, FORM_DATA_COLUMN_USED, new QTableWidgetItem("true"));
    setItem(row, FORM_DATA_COLUMN_CONTENT_TYPE, new QTableWidgetItem("text/plain"));
}

void form_data_table_widget::setItemDelegate(QAbstractItemDelegate *delegate)
{
    table_widget_delegate *widget_delgate = qobject_cast<table_widget_delegate *>(delegate);
    if(widget_delgate){
        connect(widget_delgate, &table_widget_delegate::item_editor, this, &form_data_table_widget::item_editor);
    }

    table_widget::setItemDelegate(delegate);
}


void form_data_table_widget::item_editor(const QModelIndex &index)
{
    if(index.row() != rowCount() -1 ){
        return;
    }

    int row = index.row();
    QTableWidgetItem *used_item = item(row, FORM_DATA_COLUMN_USED);
    bool used = used_item ? used_item->data(0).toBool() : false;
    if(!used && index.column() != FORM_DATA_COLUMN_USED){
       setItem(row, FORM_DATA_COLUMN_USED, new QTableWidgetItem("true"));;
    }

    QTableWidgetItem *name_item = item(row, FORM_DATA_COLUMN_NAME);
    QTableWidgetItem *content_type_item = item(row, FORM_DATA_COLUMN_CONTENT_TYPE);
    QTableWidgetItem *value_item = item(row, FORM_DATA_COLUMN_VALUE);
    if(( name_item && !name_item->data(0).toString().isEmpty())
        || ( content_type_item && !content_type_item->data(0).toString().isEmpty()
             && content_type_item->data(0).toString() != "text/plain")
        || ( value_item && !value_item->data(0).toString().isEmpty()) ){

        row = rowCount();
        setRowCount(row + 1);
        setItem(row, FORM_DATA_COLUMN_USED, new QTableWidgetItem("false"));;
    }
}


void form_data_table_widget::set_data(const http_form_data_list& datas)
{
	setRowCount(datas.size());
	for (int i = 0; i < datas.size(); i++){
		http_form_data_ptr data_ptr = datas[i];

		setItem(i, FORM_DATA_COLUMN_NAME, new QTableWidgetItem(data_ptr->name()));
        setItem(i, FORM_DATA_COLUMN_USED, new QTableWidgetItem(data_ptr->used() ? "true" : "false"));
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

        QTableWidgetItem *used_item = item(row, FORM_DATA_COLUMN_USED);
        bool used = used_item ? used_item->data(0).toBool() : true;
        data_ptr->set_used(used);

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
