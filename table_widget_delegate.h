#ifndef REST_DATA_TABLE_DELEGATE_H
#define REST_DATA_TABLE_DELEGATE_H

#include <QStyledItemDelegate>
#include <QMap>
#include <QPen>

QT_BEGIN_NAMESPACE
class QTableWidget;
QT_END_NAMESPACE

class table_widget_delegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	enum {
		WIDGET_CHECKBOX,
		WIDGET_COMBOBOX,
	};

	struct column_data_t{
		int type;
		QVariant data;
	};

	typedef QSharedPointer<column_data_t>  column_data_ptr;
	typedef QMap<int, column_data_ptr> column_data_map_t;

	table_widget_delegate(const column_data_map_t& column_datas, QTableWidget *parent);
	~table_widget_delegate();
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
protected:
	void paintWidget(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paintCheckbox(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QWidget *createWidget(const QModelIndex &index, const QStyleOptionViewItem &option, QWidget *parent) const;
	void setWidgetData(const QModelIndex &index, QWidget *editor, const QVariant& data) const;
	void setWidgetModelData(const QModelIndex &index, QAbstractItemModel *model, QWidget *editor) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model,const QStyleOptionViewItem &option,
		const QModelIndex &index);
	bool editorEventWidget(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
		const QModelIndex &index);
	bool editorEventCheckbox(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
		const QModelIndex &index);

private slots:
	void commitAndCloseEditor();

Q_SIGNALS:
    void item_editor(const QModelIndex &index);

private:
	column_data_map_t _column_datas;
	QPen _pen;
};

#endif // REST_DATA_TABLE_DELEGATE_H
