#include "table_widget_delegate.h"
#include <QtGui>
#include <QTimeEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QStyle>
#include <QTableWidget>
#include <QApplication>
#include <QHBoxLayout>

table_widget_delegate::table_widget_delegate(const column_data_map_t& column_datas,
	QTableWidget* parent)
	: QStyledItemDelegate(parent)
	, _column_datas(column_datas)
{
	int gridHint = parent->style()->styleHint(QStyle::SH_Table_GridLineColor, new QStyleOptionViewItemV4());
	QColor gridColor = static_cast<QRgb>(gridHint);
	_pen = QPen(gridColor, 0, parent->gridStyle());
}

table_widget_delegate::~table_widget_delegate()
{
}

void table_widget_delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (_column_datas.contains(index.column())) {
		paintWidget(painter, option, index);
	}
	else {
		QStyledItemDelegate::paint(painter, option, index);
	}
}

QWidget *table_widget_delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (_column_datas.contains(index.column())) {
		return createWidget(index, option, parent);
	}
	else {
		return QStyledItemDelegate::createEditor(parent, option, index);
	}
}

void table_widget_delegate::commitAndCloseEditor()
{
	QTimeEdit *editor = qobject_cast<QTimeEdit *>(sender());
	emit commitData(editor);
	emit closeEditor(editor);
}

void table_widget_delegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	if (_column_datas.contains(index.column())) {
		setWidgetData(index, editor, index.model()->data(index, Qt::DisplayRole));
	}
	else {
		QStyledItemDelegate::setEditorData(editor, index);
	}
}

void table_widget_delegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	if (_column_datas.contains(index.column())) {
		setWidgetModelData(index, model, editor);
	}
	else {
		QStyledItemDelegate::setModelData(editor, model, index);
	}
}

static QRect CheckBoxRect(const QStyleOptionViewItem &view_item_style_options)
{
	QStyleOptionButton check_box_style_option;
	QRect check_box_rect = QApplication::style()->subElementRect(
		QStyle::SE_CheckBoxIndicator,
		&check_box_style_option);

	QPoint check_box_point(view_item_style_options.rect.x() +
		view_item_style_options.rect.width() / 2 -
		check_box_rect.width() / 2,
		view_item_style_options.rect.y() +
		view_item_style_options.rect.height() / 2 -
		check_box_rect.height() / 2);
	return QRect(check_box_point, check_box_rect.size());
}

void table_widget_delegate::paintWidget(QPainter *painter, const QStyleOptionViewItem &option, 
	const QModelIndex &index) const
{
	column_data_ptr cd = _column_datas[index.column()];
	if (!cd){
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	switch (cd->type)
	{
	case WIDGET_CHECKBOX:
		paintCheckbox(painter, option, index);
		break;
	default:
		QStyledItemDelegate::paint(painter, option, index);
		break;
	}
}


void table_widget_delegate::paintCheckbox(QPainter *painter, const QStyleOptionViewItem &option,
	const QModelIndex &index) const
{
	bool checked = index.model()->data(index, Qt::EditRole).toBool();

	QStyleOptionButton check_box_style_option;
	check_box_style_option.state |= QStyle::State_Enabled;
	if (checked){
		check_box_style_option.state |= QStyle::State_On;
	}
	else{
		check_box_style_option.state |= QStyle::State_Off;
	}

	check_box_style_option.rect = CheckBoxRect(option);
	QApplication::style()->drawControl(QStyle::CE_CheckBox, &check_box_style_option, painter);
}

QWidget * table_widget_delegate::createWidget(const QModelIndex &index, const QStyleOptionViewItem &option, 
	QWidget *parent) const
{
	column_data_ptr cd = _column_datas[index.column()];
	if (!cd){
		return 0;
	}

	switch (cd->type)
	{
	case WIDGET_CHECKBOX:
		return 0;
	case  WIDGET_COMBOBOX:
		{
			QComboBox *widget = new QComboBox(parent);
			switch (cd->data.type())
			{
			case QVariant::StringList:
				widget->addItems(cd->data.toStringList());
				break;
			case QVariant::Map:
				{
					QVariantMap map = cd->data.toMap();
					for (auto it = map.begin(); it != map.end(); ++it){
						widget->addItem(it.key(), it.value());
					}
				}
				break;
			default:
				break;
			}
			return widget;
		}
	default:
		break;
	}

	return 0;
}

void table_widget_delegate::setWidgetData(const QModelIndex &index, QWidget *editor, const QVariant& data) const
{
	if (!editor){
		return;
	}

	column_data_ptr cd = _column_datas[index.column()];
	if (!cd){
		return;
	}

	switch (cd->type)
	{
	case WIDGET_CHECKBOX:
        {
            QCheckBox *widget = qobject_cast<QCheckBox*>(editor);
            if(widget){
                widget->setChecked(data.toBool());
            }
        }
		break;
	case  WIDGET_COMBOBOX:
		{
			QComboBox *widget = qobject_cast<QComboBox*>(editor);
			switch (cd->data.type())
			{
			case QVariant::StringList:
				widget->setCurrentText(data.toString());
				break;
			case QVariant::Map:
				{
					  QVariantMap map = cd->data.toMap();
					  QString text;
					  for (auto it = map.begin(); it != map.end(); ++it){
						  if (data == it.value()){
							  text = it.key();
						  }
					  }

					  widget->setCurrentText(text);
				}
				break;
			default:
				break;
			}
		}
	default:
		break;
	}
}

void table_widget_delegate::setWidgetModelData(const QModelIndex &index, QAbstractItemModel *model, QWidget *editor) const
{
	if (!editor){
		return;
	}

	column_data_ptr cd = _column_datas[index.column()];
	if (!cd){
		return;
	}

	switch (cd->type)
	{
	case WIDGET_CHECKBOX:
        {
            QCheckBox *widget = qobject_cast<QCheckBox*>(editor);
            if(widget){
                model->setData(index, widget->checkState() == Qt::Checked);
            }
        }
		break;
	case  WIDGET_COMBOBOX:
		{
			QComboBox *widget = qobject_cast<QComboBox*>(editor);
			switch (cd->data.type())
			{
			case QVariant::StringList:
				model->setData(index, widget->currentText());
				break;
			case QVariant::Map:
				{
					QVariantMap map = cd->data.toMap();
					QString text = widget->currentText();
					model->setData(index, map[text]);
				}
				break;
			default:
				break;
			}
		}
	default:
		break;
	}

}

bool table_widget_delegate::editorEvent(QEvent *event, QAbstractItemModel *model, 
	const QStyleOptionViewItem &option, const QModelIndex &index)
{
	if (_column_datas.contains(index.column())) {
		return editorEventWidget(event, model, option, index);
	}
	else {
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}
}

bool table_widget_delegate::editorEventWidget(QEvent *event, QAbstractItemModel *model,
	const QStyleOptionViewItem &option, const QModelIndex &index)
{
	column_data_ptr cd = _column_datas[index.column()];
	if (!cd){
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}

	switch (cd->type)
	{
	case WIDGET_CHECKBOX:
		return editorEventCheckbox(event, model, option, index);
		break;
	default:
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}
}

bool table_widget_delegate::editorEventCheckbox(QEvent *event, QAbstractItemModel *model,
	const QStyleOptionViewItem &option, const QModelIndex &index)
{

	if ((event->type() == QEvent::MouseButtonRelease)
		|| (event->type() == QEvent::MouseButtonDblClick))
	{
		QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
		if (mouse_event->button() != Qt::LeftButton ||
			!CheckBoxRect(option).contains(mouse_event->pos())){
			return false;
		}

		if (event->type() == QEvent::MouseButtonDblClick){
			return true;
		}
	}
	else if (event->type() == QEvent::KeyPress){
		if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space &&
			static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select){
			return false;
		}
	}
	else{
		return false;
	}

	bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
	return model->setData(index, !checked, Qt::DisplayRole);
}
