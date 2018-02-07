#ifndef TABLE_WIDGET_H
#define TABLE_WIDGET_H

#include <QTableWidget>

class table_widget : public QTableWidget
{
	Q_OBJECT

public:
	table_widget(QWidget *parent);
	~table_widget();
	bool swap_row(int src_row, int dest_row);
	bool move_row_down(int row);
	bool move_row_up(int row);

public slots:
	void move_selected_row_down();
	void move_selected_row_up();
    void insert();
    void remove_selected();

private:
	
};

#endif // TABLE_WIDGET_H
