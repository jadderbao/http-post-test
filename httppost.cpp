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

httppost::httppost(QWidget *parent)
	: QMainWindow(parent)
	, last_file_name("data")
{
	ui.setupUi(this);

	am = new QNetworkAccessManager(this);
	cookie_jar = new QNetworkCookieJar(this);
	am->setCookieJar(cookie_jar);

	connect(ui.pushButtonPost, &QPushButton::clicked, this, &httppost::post);
	connect(ui.pushButtonGet, &QPushButton::clicked, this, &httppost::get);

	connect(ui.pushButtonInsert, &QPushButton::clicked, this, &httppost::insert);
	connect(ui.pushButtonRemove, &QPushButton::clicked, this, &httppost::remove);
	connect(ui.pushButtonSave, &QPushButton::clicked, this, &httppost::save);
	connect(ui.pushButtonLoad, &QPushButton::clicked, this, &httppost::load);

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

	//设置Value列宽度
	ui.tableWidgetKeyValue->setColumnWidth(1, 380);

	//设置Value列宽度
	ui.tableWidgetHeaders->setColumnWidth(1, 380);

	//设置Value列宽度
	ui.tableWidgetResponseHeaders->setColumnWidth(1, 380);

	//设置Value列宽度
	ui.tableWidgetFormData->setColumnWidth(1, 380);


	load_url_encode_post_data("default.hpd");

	progress_bar = new QProgressBar;
	progress_bar->setVisible(false);
	ui.statusBar->addPermanentWidget(progress_bar);

	ui.tabWidgetPostBody->setCurrentIndex(0);
	ui.statusBar->showMessage("Jadder Http 测试工具 V 0.3 build 2016.3.5 e-mail: JadderBao@163.com");
}

httppost::~httppost()
{

}


void httppost::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if (bytesReceived == 0){
		progress_bar->setVisible(true);
		progress_bar->setMaximum(bytesTotal);
		progress_bar->setTextVisible(true);
	}

	progress_bar->setValue(bytesReceived);
}


void httppost::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
	if (bytesSent == 0){
		progress_bar->setVisible(true);
		progress_bar->setMaximum(bytesTotal);
		progress_bar->setTextVisible(true);
	}

	progress_bar->setValue(bytesSent);
}


void httppost::finished()
{
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

    QHttpMultiPart *multi_part = 0;
	QString body;
	switch (ui.tabWidgetPostBody->currentIndex()){
	case 0:
		body = get_url_encode_body();
		break;
	case 1:
		multi_part = new QHttpMultiPart();
		form_data_table_to_multi_part(multi_part, ui.tableWidgetFormData);
		break;
	case 2:
		body = ui.plainTextEditJson->toPlainText();
		break;
	case 3:
		body = ui.plainTextEditText->toPlainText();
		break;
	default:
		break;
	}

	while (true)
	{
		QNetworkRequest request(url);
		QEventLoop loop;
        QNetworkReply *reply = 0;

		QSslConfiguration config;
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		config.setProtocol(QSsl::TlsV1_0OrLater);
		request.setSslConfiguration(config);

		switch (ui.tabWidgetPostBody->currentIndex()){
		case 0:
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
			update_request_custom_header(&request);
			reply = am->post(request, body.toLocal8Bit());
			break;
		case 1:
            request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");
			update_request_custom_header(&request);
			reply = am->post(request, multi_part);
			break;
		case 2:
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
			update_request_custom_header(&request);
			reply = am->post(request, body.toLocal8Bit());
			break;
		case 3:
            request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");
			update_request_custom_header(&request);
			reply = am->post(request, body.toLocal8Bit());
			break;
		default:
			break;
		}

		connect(reply, &QNetworkReply::downloadProgress, this, &httppost::downloadProgress);
		connect(reply, &QNetworkReply::uploadProgress, this, &httppost::uploadProgress);
		connect(reply, &QNetworkReply::finished, this, &httppost::finished);
		connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
		loop.exec();

		int http_status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (http_status == 302){
			url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

			reply->close();
			reply->deleteLater();
			continue;
		}

		show_reply(reply);
		break;
	}

	if (multi_part){
		delete multi_part;
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

	if (ui.tabWidgetPostBody->currentIndex() != 0){
		ui.tabWidgetPostBody->setCurrentIndex(0);
	}

	QString body = get_url_encode_body();
	url.setQuery(url.hasQuery() ? url.query() + "&" + body : body);

	while (true)
	{
		QNetworkRequest request(url);

		QSslConfiguration config;
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		config.setProtocol(QSsl::TlsV1_0OrLater);
		request.setSslConfiguration(config);

		update_request_custom_header(&request);

		QEventLoop loop;
		QNetworkReply *reply = am->get(request);

		connect(reply, &QNetworkReply::downloadProgress, this, &httppost::downloadProgress);
		connect(reply, &QNetworkReply::uploadProgress, this, &httppost::uploadProgress);
		connect(reply, &QNetworkReply::finished, this, &httppost::finished);
		connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
		loop.exec();

		int http_status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (http_status == 302){
			url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

			reply->close();
			reply->deleteLater();
			continue;
		}

		show_reply(reply);
		break;
	}
}

void httppost::show_reply(QNetworkReply * reply)
{
	if (reply->error() == QNetworkReply::NoError)
	{
		//没有错误，输出返回值
		QByteArray bytes = reply->readAll();  // bytes 
		QTextCodec * codec = QTextCodec::codecForHtml(bytes);
		QString response_body = codec->toUnicode(bytes.data());

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

QString httppost::get_url_encode_body()
{
	QString body;
	int count = ui.tableWidgetKeyValue->rowCount();
	for (int i = 0; i < count; i++){
		QString key = ui.tableWidgetKeyValue->item(i, 0)->data(0).toString();
		body.append(url_encode(key));
		body.append("=");

		QString value = ui.tableWidgetKeyValue->item(i, 1)->data(0).toString();
		body.append(url_encode(value));
		body.append("&");
	}

	if (!body.isEmpty()){
		body.remove(body.size() - 1, 1);
	}

	return body;
}

QByteArray httppost::url_encode(QString &data)
{
	QByteArray ba = data.toUtf8();
	return ba.toPercentEncoding();
}

void httppost::insert()
{
	ui.tableWidgetKeyValue->setRowCount(ui.tableWidgetKeyValue->rowCount() + 1);
}

void httppost::remove()
{
	ui.tableWidgetKeyValue->removeRow(ui.tableWidgetKeyValue->currentIndex().row());
}

void httppost::save()
{
	QString file_name = get_save_file_name("Http Post Data(*.hpd)");
	if (file_name.isEmpty()){
		return;
	}

	save_url_encode_post_data(file_name);
	return;
}


void httppost::load()
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

	QFile file(file_name);
	if (!file.open(QIODevice::ReadWrite)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return;
	}

	//table 转json 格式
	QJsonObject v = key_value_table_to_json(ui.tableWidgetFormData);
	v["url"] = ui.lineEditUrl->text();
	v["headers"] = key_value_table_to_json(ui.tableWidgetHeaders);

	QJsonDocument doc(v);
	doc.toJson();

	file.write(doc.toJson());
	return;

}


void httppost::form_load()
{
	QString file_name = get_open_file_name("Http Post Form Data(*.hpfd)");
	if (file_name.isEmpty()){
		return;
	}

	QFile file(file_name);
	if (!file.open(QIODevice::ReadWrite)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return;
	}

	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
	if (doc.isNull()){
		QMessageBox::warning(this, "Json 错误", error.errorString(), QMessageBox::Ok);
		return;
	}

	if (!doc.isObject()){
		QMessageBox::warning(this, "Json 错误", "json数据不是本软件需要的。", QMessageBox::Ok);
		return;
	}

	QJsonObject v = doc.object();
	ui.lineEditUrl->setText(v["url"].toString());
	key_value_json_to_table(v, ui.tableWidgetFormData);

	QJsonObject h = v["headers"].toObject();
	key_value_json_to_table(h, ui.tableWidgetHeaders);
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

	QFile file(file_name);
	if (!file.open(QIODevice::ReadWrite)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return;
	}

	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
	if (doc.isNull()){
		QMessageBox::warning(this, "Json 错误", error.errorString(), QMessageBox::Ok);
		return;
	}

	if (!doc.isObject()){
		QMessageBox::warning(this, "Json 错误", "json数据不是本软件需要的。", QMessageBox::Ok);
		return;
	}

	QJsonObject v = doc.object();
	key_value_json_to_table(v, ui.tableWidgetHeaders);
	return;
}


void httppost::header_save()
{
	QString file_name = get_save_file_name("Http Header Data(*.hhd)");
	if (file_name.isEmpty()){
		return;
	}

	QFile file(file_name);
	if (!file.open(QIODevice::ReadWrite)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return;
	}

	//table 值转json object
	QJsonObject v = key_value_table_to_json(ui.tableWidgetHeaders);

	QJsonDocument doc(v);
	file.write(doc.toJson());
}


QJsonObject httppost::key_value_table_to_json(QTableWidget *tableWidget)
{
	QJsonArray keys, values;
	for (int row = 0; row < tableWidget->rowCount(); row++){
		QTableWidgetItem *key_item = tableWidget->item(row, 0);
		QString key = key_item ? key_item->data(0).toString() : "";
		keys.append(key);

		QTableWidgetItem *value_item = tableWidget->item(row, 1);
		QString value = value_item ? value_item->data(0).toString() : "";
		values.append(value);
	}

	QJsonObject v;
	v["keys"] = keys;
	v["values"] = values;

	return v;
}

void httppost::key_value_json_to_table(QJsonObject &v, QTableWidget *tableWidget)
{
	QJsonArray keys = v["keys"].toArray();
	QJsonArray values = v["values"].toArray();
	if (keys.count() != values.count()){
		QMessageBox::warning(this, "Json数据错误", "json数据中Key-Value 数据不一致。", QMessageBox::Ok);
		return;
	}

	tableWidget->setRowCount(keys.count());
	for (int i = 0; i < keys.size(); i++){
		tableWidget->setItem(i, 0, new QTableWidgetItem(keys[i].toString()));
		tableWidget->setItem(i, 1, new QTableWidgetItem(values[i].toString()));
	}
}


void httppost::form_data_table_to_multi_part(QHttpMultiPart* multi_part,
	QTableWidget *tableWidget)
{
	for (int row = 0; row < tableWidget->rowCount(); row++){
		QTableWidgetItem *key_item = tableWidget->item(row, 0);
		QString key = key_item ? key_item->data(0).toString() : "";
		if (key.isEmpty()){
			continue;
		}

		QTableWidgetItem *value_item = tableWidget->item(row, 1);
		QString value = value_item ? value_item->data(0).toString() : "";

		if (!QFile::exists(value)){
			QHttpPart textPart;
			textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"text\""));
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
			part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(key));
			part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\""+ file->fileName() +"\""));
			part.setBodyDevice(file);
			multi_part->append(part);
		}

	}
}

void httppost::update_request_custom_header(QNetworkRequest *request)
{
	QTableWidget *table_widget = ui.tableWidgetHeaders;
	for (int row = 0; row < table_widget->rowCount(); row++){
		QTableWidgetItem *key_item = table_widget->item(row, 0);
		QString key = key_item ? key_item->data(0).toString() : "";
		if (key.isEmpty()){
			continue;
		}

		QTableWidgetItem *value_item = table_widget->item(row, 1);
		QString value = value_item ? value_item->data(0).toString() : "";

		request->setRawHeader(key.toUtf8(), value.toUtf8());
	}
}


void httppost::insert_form_data(const QString& type, const QString& file_name)
{
	if (file_name.isEmpty()){
		return;
	}

	int row = ui.tableWidgetFormData->rowCount();
	ui.tableWidgetFormData->setRowCount(row + 1);
	ui.tableWidgetFormData->setItem(row, 0, new QTableWidgetItem(type));
	ui.tableWidgetFormData->setItem(row, 1, new QTableWidgetItem(file_name));
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
	if (!file.open(QIODevice::ReadWrite)){
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
	if (!file.open(QIODevice::ReadWrite)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return;
	}

	file.write(data);
}

void httppost::save_url_encode_post_data(const QString& file_name)
{
	QFile file(file_name);
	if (!file.open(QIODevice::ReadWrite)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return;
	}

	//table 转json 格式
	QJsonObject v = key_value_table_to_json(ui.tableWidgetKeyValue);
	v["url"] = ui.lineEditUrl->text();
	v["headers"] = key_value_table_to_json(ui.tableWidgetHeaders);

	QJsonDocument doc(v);
	doc.toJson();

	file.write(doc.toJson());
}

void httppost::load_url_encode_post_data(const QString& file_name)
{
	if (!QFile::exists(file_name)){
		return;
	}

	QFile file(file_name);
	if (!file.open(QIODevice::ReadWrite)){
		QMessageBox::warning(this, "文件打开错误", file.errorString(), QMessageBox::Ok);
		return;
	}

	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
	if (doc.isNull()){
		QMessageBox::warning(this, "Json 错误", error.errorString(), QMessageBox::Ok);
		return;
	}

	if (!doc.isObject()){
		QMessageBox::warning(this, "Json 错误", "json数据不是本软件需要的。", QMessageBox::Ok);
		return;
	}

	QJsonObject v = doc.object();
	ui.lineEditUrl->setText(v["url"].toString());
	key_value_json_to_table(v, ui.tableWidgetKeyValue);

	QJsonObject h = v["headers"].toObject();
	key_value_json_to_table(h, ui.tableWidgetHeaders);
}
