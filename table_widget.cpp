#include "table_widget.h"

table_widget::table_widget(QWidget *parent)
	: QTableWidget(parent)
{

}

table_widget::~table_widget()
{

}

bool table_widget::swap_row(int src_row, int dest_row)
{
	if (src_row < 0 || src_row >= rowCount()
		|| dest_row < 0 || dest_row >= rowCount()
		|| src_row == dest_row ){
		return false;
	}

	for (int column = 0; column < columnCount(); column++){
		QTableWidgetItem *dest_item = item(dest_row, column);
		QVariant dest_data = dest_item ? dest_item->data(Qt::DisplayRole) : QVariant();

		QTableWidgetItem *src_item = item(src_row, column);
		QVariant src_data = src_item ? src_item->data(Qt::DisplayRole) : QVariant();

		if (dest_item){
			dest_item->setData(Qt::DisplayRole, src_data);
		}
		else {
			QTableWidgetItem *item = new QTableWidgetItem();
			item->setData(Qt::DisplayRole, src_data);
			setItem(dest_row, column, item);
		}

		if (src_item){
			src_item->setData(Qt::DisplayRole, dest_data);
		}
		else {
			QTableWidgetItem *item = new QTableWidgetItem();
			item->setData(Qt::DisplayRole, dest_data);
			setItem(src_row, column, item);
		}
	}

	return true;
}

bool table_widget::move_row_down(int row)
{
	if (row < 0 || row >= rowCount() - 1){
		return false;
	}

	return swap_row(row, row + 1);
}

bool table_widget::move_row_up(int row)
{
	if (row <= 0 || row >= rowCount()){
		return false;
	}

	return swap_row(row, row - 1);
}

void table_widget::move_selected_row_down()
{
	QModelIndex index = currentIndex();
	if (move_row_down(index.row())){
	}
}

void table_widget::move_selected_row_up()
{
	QModelIndex index = currentIndex();
    move_row_up(index.row());
}

void table_widget::insert()
{
    int row = rowCount();
    setRowCount( row + 1);
}

void table_widget::remove_selected()
{
    removeRow(currentIndex().row());
}
