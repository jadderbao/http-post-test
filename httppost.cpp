#include "httppost.h"
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QProgressBar>
#include <QHttpMultiPart>

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextCodec>
#include <QFileDialog>

#include <QSsl>  
#include <QSslConfiguration>  
#include <QSslSocket> 
#include <QMap>
#include <QScriptEngine>

#include "table_widget_delegate.h"
#include "http_data.h"
#include "http_client.h"
#include "http_script_engine.h"
#include "post_data_process.h"

httppost::httppost(QWidget *parent)
	: QMainWindow(parent)
	, last_file_name("data")
{
	ui.setupUi(this);
	
	_engine = new http_script_engine(this);
	initialize_script_engine(_engine);

	am = new QNetworkAccessManager(this);
	cookie_jar = new QNetworkCookieJar(this);
	am->setCookieJar(cookie_jar);

	connect(ui.pushButtonPost, &QPushButton::clicked, this, &httppost::post);
	connect(ui.pushButtonGet, &QPushButton::clicked, this, &httppost::get);

	connect(ui.pushButtonInsert, &QPushButton::clicked, this, &httppost::url_encode_body_insert);
	connect(ui.pushButtonRemove, &QPushButton::clicked, this, &httppost::url_encode_body_remove);
	connect(ui.pushButtonSave, &QPushButton::clicked, this, &httppost::url_encode_body_save);
	connect(ui.pushButtonLoad, &QPushButton::clicked, this, &httppost::url_encode_body_load);

	connect(ui.pushButtonFormInsertText, &QPushButton::clicked, this, &httppost::form_add_text);
	connect(ui.pushButtonFormInsertImage, &QPushButton::clicked, this, &httppost::form_add_image);
	connect(ui.pushButtonFormInsertRaw, &QPushButton::clicked, this, &httppost::form_add_raw);

	connect(ui.pushButtonFormInsert, &QPushButton::clicked, this, &httppost::form_insert);
	connect(ui.pushButtonFormRemove, &QPushButton::clicked, this, &httppost::form_remove);
	connect(ui.pushButtonFormLoad, &QPushButton::clicked, this, &httppost::form_load);
	connect(ui.pushButtonFormSave, &QPushButton::clicked, this, &httppost::form_save);

	connect(ui.pushButtonJsonLoad, &QPushButton::clicked, this, &httppost::json_load);
	connect(ui.pushButtonJsonSave, &QPushButton::clicked, this, &httppost::json_save);

	connect(ui.pushButtonTextLoad, &QPushButton::clicked, this, &httppost::text_load);
	connect(ui.pushButtonTextSave, &QPushButton::clicked, this, &httppost::text_save);

	connect(ui.pushButtonHeaderInsert, &QPushButton::clicked, this, &httppost::header_insert);
	connect(ui.pushButtonHeaderRemove, &QPushButton::clicked, this, &httppost::header_remove);
	connect(ui.pushButtonHeaderSave, &QPushButton::clicked, this, &httppost::header_save);
	connect(ui.pushButtonHeaderLoad, &QPushButton::clicked, this, &httppost::header_load);

	set_table_widgets();

	//
    ui.checkBoxAutoUCenterAuthentication->hide();

	load_url_encode_post_data("default.hpd");

	progress_bar = new QProgressBar;
	progress_bar->setVisible(false);
	progress_bar->setMaximum(0);
	ui.statusBar->addPermanentWidget(progress_bar);

	ui.tabWidgetPostBody->setCurrentIndex(0);
	ui.statusBar->showMessage("Jadder Http 测试工具 V 0.4 build 2016.3.5 e-mail: JadderBao@163.com");
}

httppost::~httppost()
{

}


void httppost::set_table_widgets()
{
	ui.tableWidgetUrlEncode->setEditTriggers(QAbstractItemView::AllEditTriggers);
	ui.tableWidgetHeaders->setEditTriggers(QAbstractItemView::AllEditTriggers);
	ui.tableWidgetFormData->setEditTriggers(QAbstractItemView::AllEditTriggers);

	//设置Value列宽度
	ui.tableWidgetResponseHeaders->setColumnWidth(1, 600);

	table_widget_delegate::column_data_map_t map;

	table_widget_delegate::column_data_ptr post_type_column_data(table_widget_delegate::column_data_ptr::create());
	post_type_column_data->type = table_widget_delegate::WIDGET_COMBOBOX;
	post_type_column_data->data = QStringList() << POST_TYPE_NONE_STR << POST_TYPE_HEADER_STR;

	map.insert(url_encode_table_widget::URL_ENCODE_COLUMN_POST_TYPE, post_type_column_data);

	table_widget_delegate::column_data_ptr auth_column_data(table_widget_delegate::column_data_ptr::create());
	auth_column_data->type = table_widget_delegate::WIDGET_CHECKBOX;

	map.insert(url_encode_table_widget::URL_ENCODE_COLUMN_AUTH, auth_column_data);

	table_widget_delegate::column_data_ptr value_type_column_data(table_widget_delegate::column_data_ptr::create());
	value_type_column_data->type = table_widget_delegate::WIDGET_COMBOBOX;
	value_type_column_data->data = QStringList() << POST_VALUE_TYPE_VALUE_STR << POST_VALUE_TYPE_UCENTER_AUTHCODE_STR
		<< POST_VALUE_TYPE_JAVASCRIPT_STR;

	map.insert(url_encode_table_widget::URL_ENCODE_COLUMN_VALUE_TYPE, value_type_column_data);

	table_widget_delegate * headers_delgate = new table_widget_delegate(map, ui.tableWidgetHeaders);
	ui.tableWidgetHeaders->setItemDelegate(headers_delgate);

	table_widget_delegate::column_data_ptr header_post_type_column_data(table_widget_delegate::column_data_ptr::create());
	header_post_type_column_data->type = table_widget_delegate::WIDGET_COMBOBOX;
	header_post_type_column_data->data = QStringList() << POST_TYPE_NONE_STR << POST_TYPE_URL_QUERY_STR
		<< POST_TYPE_BODY_STR << POST_TYPE_HEADER_STR;

	map[url_encode_table_widget::URL_ENCODE_COLUMN_POST_TYPE] = header_post_type_column_data;

	table_widget_delegate * url_encode_delgate = new table_widget_delegate(map, ui.tableWidgetUrlEncode);
	ui.tableWidgetUrlEncode->setItemDelegate(url_encode_delgate);

}


void httppost::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if (progress_bar->maximum() == 0){
		progress_bar->setMaximum(bytesTotal);
		progress_bar->setVisible(true);
		progress_bar->setTextVisible(true);
	}

	progress_bar->setValue(bytesReceived);
}


void httppost::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
	if (progress_bar->maximum() == 0){
		progress_bar->setVisible(true);
		progress_bar->setMaximum(bytesTotal);
		progress_bar->setTextVisible(true);
	}

	progress_bar->setValue(bytesSent);
}


void httppost::finished()
{
	progress_bar->setMaximum(0);
	progress_bar->setVisible(false);
}

void httppost::post()
{
	QUrl url(ui.lineEditUrl->text());
	if (!url.isValid()){
		QMessageBox::warning(this, "Url", "无效的Url", QMessageBox::Ok);
		ui.lineEditUrl->setFocus();
		return;
	}

	//设置Headers 页面，无法确定Post 类型。
	if (ui.tabWidgetPostBody->currentIndex() == 4){
		QMessageBox::warning(this, "无法确定POST类型", "请选择相应的属性页（0-4）, 不要选择“头”页。", QMessageBox::Ok);
		ui.tabWidgetPostBody->setCurrentIndex(0);
		return;
	}

	switch (ui.tabWidgetPostBody->currentIndex()){
	case 0: // url_encoded
		post_url_encoded(url);
		break;
	case 1:
		post_multi_part(url);
		break;
	case 2: //json
		post_byte_data(url, ui.plainTextEditJson->toPlainText().toUtf8());
		break;
	case 3: //text/xml
		post_byte_data(url, ui.plainTextEditText->toPlainText().toUtf8());
		break;
	default:
		break;
	}
}


void httppost::get()
{

	QUrl url(ui.lineEditUrl->text());
	if (!url.isValid()){
		QMessageBox::warning(this, "Url", "无效的Url", QMessageBox::Ok);
		ui.lineEditUrl->setFocus();
		return;
	}

	//只有当前项为url_encoded 页时才处理get
	if (ui.tabWidgetPostBody->currentIndex() != 0){
		ui.tabWidgetPostBody->setCurrentIndex(0);
	}

	http_data_list items;
	if (!get_ui_data_items(items)){
		return;
	}

	//原来 url, body 上的参数都一起发送
	QString body = items.to_url_encode_string(http_data::POST_TYPE_BODY | http_data::POST_TYPE_URL_QUERY);
	url.setQuery(url.hasQuery() ? url.query() + "&" + body : body);

	QSharedPointer<http_client> client(new http_client(am));

	//
	QSharedPointer<http_request> request(new http_request(url));
	update_request_custom_header(items, request->request());

	connect(client.data(), &http_client::downloadProgress, this, &httppost::downloadProgress);
	connect(client.data(), &http_client::uploadProgress, this, &httppost::uploadProgress);
	connect(client.data(), &http_client::finished, this, &httppost::finished);

	QNetworkReply *reply = client->get(request.data());
	show_reply(reply);
}

void httppost::show_reply(QNetworkReply * reply)
{
	if (reply->error() == QNetworkReply::NoError)
	{
		//没有错误，输出返回值
		QString response_body = reply->readAll();  // bytes 
		QJsonParseError error;
		QJsonDocument doc = QJsonDocument::fromJson(response_body.toUtf8(), &error);
		if (error.error != QJsonParseError::NoError){
			ui.plainTextEditResponseBody->setPlainText(response_body);
		}
		else{
			ui.plainTextEditResponseBody->setPlainText(doc.toJson());
		}
	}
	else
	{
		//如果有错误，显示出错信息
		ui.plainTextEditResponseBody->setPlainText(reply->errorString());
	}

	//输出头
	const QList<QNetworkReply::RawHeaderPair>& headers = reply->rawHeaderPairs();
	ui.tableWidgetResponseHeaders->setRowCount(headers.count());
	for (int i = 0; i < headers.size(); i++){
		ui.tableWidgetResponseHeaders->setItem(i, 0, 
			new QTableWidgetItem(QString(headers[i].first)));
		ui.tableWidgetResponseHeaders->setItem(i, 1,
			new QTableWidgetItem(QString(headers[i].second)));
	}

	reply->close();
	reply->deleteLater();
}

void httppost::add_post_data_item(http_data_list& items, http_data_ptr item)
{
	items.append(item);
}


bool httppost::get_ui_data_items(http_data_list& items)
{
	//取自定义头数据
	items.append(ui.tableWidgetHeaders->data());
	items.append(ui.tableWidgetUrlEncode->data());

	QString error;
	if (!process_and_update_item_value(items, error)){
		QMessageBox::warning(this, "无效的数据项", "错误:" + error, QMessageBox::Ok);
		return false;
	}

	return true;
}

bool httppost::process_and_update_item_value(http_data_list& items, QString& error)
{
	post_data_process process(_engine, 0);
	bool ok = process.process_post_data(&items);
	if (!ok){
		error = process.error_string();
	}
	return ok;
}

void httppost::http_post(QUrl &url, http_request_body * http_body, const http_data_list &items)
{
	//添加Url query部分
	QString url_data = items.to_url_encode_string(http_data::POST_TYPE_URL_QUERY);
	url.setQuery(url.hasQuery() ? url.query() + "&" + url_data : url_data);

	QSharedPointer<http_client> client(new http_client(am));
	QSharedPointer<http_request> request(new http_request(url));

	request->set_body(http_body);
	update_request_custom_header(items, request->request());

	connect(client.data(), &http_client::downloadProgress, this, &httppost::downloadProgress);
	connect(client.data(), &http_client::uploadProgress, this, &httppost::uploadProgress);
	connect(client.data(), &http_client::finished, this, &httppost::finished);

	QNetworkReply *reply = client->post(request.data());
	show_reply(reply);
}

void httppost::http_post(QUrl &url, QHttpMultiPart *multi_part, const http_data_list &items)
{
	http_request_body *http_body = new http_request_body(multi_part);
	http_post(url, http_body, items);
}

void httppost::http_post(QUrl &url, const QByteArray &body, const http_data_list &items)
{
	http_request_body *http_body = new http_request_body(body);
	http_post(url, http_body, items);
}

void httppost::post_url_encoded(QUrl& url)
{
	http_data_list items;

	if (!get_ui_data_items(items)){
		return;
	}

	QString body;
	body = items.to_url_encode_string(http_data::POST_TYPE_BODY);
	http_post(url, body.toUtf8(), items);
}

void httppost::post_byte_data(QUrl& url, const QByteArray& data)
{
	http_data_list items;

	if (!get_ui_data_items(items)){
		return;
	}

	http_post(url, data, items);
}

void httppost::post_multi_part(QUrl& url)
{
	http_data_list items;
	if (!get_ui_data_items(items)){
		return;
	}

	QHttpMultiPart *multi_part = new QHttpMultiPart();
	form_data_table_to_multi_part(multi_part, ui.tableWidgetFormData);
	http_post(url, multi_part, items);
}

void httppost::url_encode_body_insert()
{
	int row = ui.tableWidgetUrlEncode->rowCount();
	ui.tableWidgetUrlEncode->setRowCount( row + 1);
}

void httppost::url_encode_body_remove()
{
	ui.tableWidgetUrlEncode->removeRow(ui.tableWidgetUrlEncode->currentIndex().row());
}

void httppost::url_encode_body_save()
{
	QString file_name = get_save_file_name("Http Post Data(*.hpd)");
	if (file_name.isEmpty()){
		return;
	}

	save_url_encode_post_data(file_name);
	return;
}


void httppost::url_encode_body_load()
{
	QString file_name = get_open_file_name("Http Post Data(*.hpd)");
	if (file_name.isEmpty()){
		return;
	}

	load_url_encode_post_data(file_name);
	return;
}


void httppost::form_add_text()
{
	QString fileName = get_open_file_name(tr("Image Files (*.txt *.log *.xml *.json)"));
	insert_form_data("text/xml", fileName);
}


void httppost::form_add_image()
{
	QString fileName = get_open_file_name(tr("Image Files (*.png *.jpg *.bmp)"));
	insert_form_data("image/jpeg", fileName);
}


void httppost::form_add_raw()
{
	QString fileName = get_open_file_name(tr("All Files (*.*)"));
	insert_form_data("application/octet-stream", fileName);
}


void httppost::form_insert()
{
	ui.tableWidgetFormData->setRowCount(ui.tableWidgetFormData->rowCount() + 1);
}

void httppost::form_remove()
{
	ui.tableWidgetFormData->removeRow(ui.tableWidgetFormData->currentIndex().row());
}

void httppost::form_save()
{
	QString file_name = get_save_file_name("Http Post Form Data(*.hpfd)");
	if (file_name.isEmpty()){
		return;
	}

	//table 转json 格式
	QJsonObject v;
	v["form"] = ui.tableWidgetFormData->data().to_json();
	v["url"] = ui.lineEditUrl->text();
	v["headers"] = ui.tableWidgetHeaders->data().to_json();

	save_json_file(file_name, v);
}


void httppost::form_load()
{
	QString file_name = get_open_file_name("Http Post Form Data(*.hpfd)");
	if (file_name.isEmpty()){
		return;
	}

	QJsonObject v = load_json_file(file_name);
	ui.lineEditUrl->setText(v["url"].toString());
	QJsonValue form = v["form"];
	ui.tableWidgetFormData->set_data(http_form_data_list::from_json(form));

	QJsonValue h = v["headers"];
	ui.tableWidgetHeaders->set_data(http_data_list::from_json(v));
}


void httppost::json_save()
{
	save_text_file("Json File(*.json)", ui.plainTextEditJson->toPlainText().toLocal8Bit());
}

void httppost::json_load()
{
	QByteArray data = load_text_file("Json File(*.json)");
	if (data.isEmpty()){
		return;
	}

	ui.plainTextEditJson->setPlainText(data);
}


void httppost::text_save()
{
	save_text_file("Xml File(*.xml)", ui.plainTextEditText->toPlainText().toLocal8Bit());
}


void httppost::text_load()
{
	QByteArray data = load_text_file("Xml File(*.xml)");
	if (data.isEmpty()){
		return;
	}

	ui.plainTextEditText->setPlainText(data);
}


void httppost::header_insert()
{
	ui.tableWidgetHeaders->setRowCount(ui.tableWidgetHeaders->rowCount() + 1);
}

void httppost::header_remove()
{
	ui.tableWidgetHeaders->removeRow(ui.tableWidgetHeaders->currentIndex().row());
}

void httppost::header_load()
{
	QString file_name = get_open_file_name("Http Header Data(*.hhd)");
	if (file_name.isEmpty()){
		return;
	}

	QJsonObject v = load_json_file(file_name);
	QJsonValue h = v["headers"];

	http_data_list headers;
	headers.append_from_json(h);
	ui.tableWidgetHeaders->set_data(headers);
}


void httppost::header_save()
{
	QString file_name = get_save_file_name("Http Header Data(*.hhd)");
	if (file_name.isEmpty()){
		return;
	}

	//table 值转json object
	QJsonObject v;
	v["headers"] = ui.tableWidgetHeaders->data().to_json();
	save_json_file(file_name, v);
}

void httppost::form_data_table_to_multi_part(QHttpMultiPart* multi_part,
	QTableWidget *tableWidget)
{
	for (int row = 0; row < tableWidget->rowCount(); row++){

		QTableWidgetItem *name_item = tableWidget->item(row, form_data_table_widget::FORM_DATA_COLUMN_NAME);
		QString name = name_item ? name_item->data(0).toString() : "";

		QTableWidgetItem *content_type_item = tableWidget->item(row, form_data_table_widget::FORM_DATA_COLUMN_CONTENT_TYPE);
		QString content_type = content_type_item ? content_type_item->data(0).toString() : "";
		if (content_type.isEmpty()){
			continue;
		}

		QTableWidgetItem *value_item = tableWidget->item(row, form_data_table_widget::FORM_DATA_COLUMN_VALUE);
		QString value = value_item ? value_item->data(0).toString() : "";

		if (!QFile::exists(value)){
			QHttpPart textPart;
			textPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(content_type));
			textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
				QVariant(QString("form-data; name=\"%1\"").arg(name)));
			textPart.setBody(value.toUtf8());
			multi_part->append(textPart);
		}
		else{
			QFile *file = new QFile(value);
			if (!file->open(QIODevice::ReadOnly)){
				QMessageBox::warning(0, tr("File Open Error"), file->errorString(), QMessageBox::Ok);

				delete file;
				continue;
			}

			file->setParent(multi_part); // we cannot delete the file now, so delete it with the multiPart

			QHttpPart part;
			part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(content_type));
			part.setHeader(QNetworkRequest::ContentDispositionHeader, 
				QVariant(QString("form-data; name=\"%1\"").arg(name)));
			part.setBodyDevice(file);
			multi_part->append(part);
		}

	}
}

void httppost::update_request_custom_header(const http_data_list& items, QNetworkRequest *request)
{
	for (auto it = items.begin(); it != items.end(); ++it){
		http_data_constptr pdi = *it;
		QString key = pdi->key();
		if (key.isEmpty()){
			continue;
		}

		QString value = pdi->value();
		request->setRawHeader(key.toUtf8(), value.toUtf8());
	}
}

void httppost::initialize_script_engine(http_script_engine *engine)
{
	QString startup_script("http_post_startup.js");
	QFile script_file(startup_script);
	script_file.open(QIODevice::ReadOnly);
	engine->evaluate(script_file.readAll(), startup_script);
	script_file.close();

	QScriptValue ctor = engine->evaluate("http_post");
	if (ctor.isValid() && !ctor.isError()){
		QScriptValue http_post_ui = engine->newQObject(this, QScriptEngine::ScriptOwnership);
		QScriptValue http_post = ctor.construct(QScriptValueList() << http_post_ui);
	}
}

void httppost::insert_form_data(const QString& type, const QString& file_name)
{
	if (file_name.isEmpty()){
		return;
	}

	int row = ui.tableWidgetFormData->rowCount();
	ui.tableWidgetFormData->setRowCount(row + 1);
	ui.tableWidgetFormData->setItem(row, form_data_table_widget::FORM_DATA_COLUMN_NAME, 
		new QTableWidgetItem(QFileInfo(file_name).fileName()));
	ui.tableWidgetFormData->setItem(row, form_data_table_widget::FORM_DATA_COLUMN_CONTENT_TYPE,
		new QTableWidgetItem(type));
	ui.tableWidgetFormData->setItem(row, form_data_table_widget::FORM_DATA_COLUMN_VALUE,
		new QTableWidgetItem(file_name));
}

QString httppost::get_open_file_name(const QString& filter)
{
	QString file_name = QFileDialog::getOpenFileName(0, QString(), last_file_name, filter);
	if (!file_name.isEmpty()){
		last_file_name = file_name;
	}

	return file_name;
}


QString httppost::get_save_file_name(const QString& filter)
{
	QString file_name = QFileDialog::getSaveFileName(0, QString(), last_file_name, filter);
	if (!file_name.isEmpty()){
		last_file_name = file_name;
	}
	return file_name;
}

QByteArray httppost::load_text_file(const QString& filter)
{
	QString file_name = get_open_file_name(filter);
	if (file_name.isEmpty()){
		return QByteArray();
	}

	QFile file(file_name);
	if (!file.open(QIODevice::ReadOnly)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return QByteArray();
	}

	return file.readAll();
}

void httppost::save_text_file(const QString& filter, const QByteArray& data)
{
	QString file_name = get_save_file_name(filter);
	if (file_name.isEmpty()){
		return;
	}

	QFile file(file_name);
	if (!file.open(QIODevice::WriteOnly)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return;
	}

	file.write(data);
}

void httppost::save_url_encode_post_data(const QString& file_name)
{

	//table 转json 格式
	QJsonObject v;
	v["url_encode"] =ui.tableWidgetUrlEncode->data().to_json();
	v["url"] = ui.lineEditUrl->text();
	v["headers"] = ui.tableWidgetHeaders->data().to_json();
	save_json_file(file_name, v);
}

void httppost::load_url_encode_post_data(const QString& file_name)
{
	if (!QFile::exists(file_name)){
		return;
	}

	QJsonObject v = load_json_file(file_name);

	ui.lineEditUrl->setText(v["url"].toString());
	QJsonValue url_encode = v["url_encode"];
	ui.tableWidgetUrlEncode->set_data(http_data_list::from_json(url_encode));

	QJsonValue h = v["headers"];
	ui.tableWidgetHeaders->set_data(http_data_list::from_json(h));
}

QJsonObject httppost::load_json_file(const QString& file_name)
{
	QFile file(file_name);
	if (!file.open(QIODevice::ReadOnly)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return QJsonObject();
	}

	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
	if (doc.isNull()){
		QMessageBox::warning(this, "Json 错误", error.errorString(), QMessageBox::Ok);
		return QJsonObject();
	}

	if (!doc.isObject()){
		QMessageBox::warning(this, "Json 错误", "json数据不是本软件需要的。", QMessageBox::Ok);
		return QJsonObject();
	}

	QJsonObject obj = doc.object();
	if (!obj.contains("json_file_version") || obj["json_file_version"] != JSON_FILE_VERSION){
		QMessageBox::warning(this, "Json 错误", "json数据版本不符。", QMessageBox::Ok);
		return QJsonObject();
	}

	return doc.object();
}

void httppost::save_json_file(const QString& file_name, const QJsonObject& v)
{
	QFile file(file_name);
	if (!file.open(QIODevice::WriteOnly)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return;
	}

	QJsonObject& ov = const_cast<QJsonObject&>(v);
	ov["json_file_version"] = JSON_FILE_VERSION;

	QJsonDocument doc(v);
	file.write(doc.toJson());
}

