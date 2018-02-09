﻿#include "httppost.h"
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

#define DEFAULT_FILE_NAME "default.hpd"
#define FORM_STATUS_FILE_NAME "formstatus.dat"

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

    connect(ui.pushButtonLoad, &QPushButton::clicked, this, &httppost::load);
    connect(ui.pushButtonSave, &QPushButton::clicked, this, &httppost::save);
    connect(ui.pushButtonSend, &QPushButton::clicked, this, &httppost::send);

    connect(ui.radioButtonUrlEncoded, &QRadioButton::clicked, this, &httppost::content_type_button_clicked);
    connect(ui.radioButtonFormData, &QRadioButton::clicked, this, &httppost::content_type_button_clicked);
    connect(ui.radioButtonJson, &QRadioButton::clicked, this, &httppost::content_type_button_clicked);
    connect(ui.radioButtonCustom, &QRadioButton::clicked, this, &httppost::content_type_button_clicked);

    connect(ui.pushButtonCustomOpenFile, &QPushButton::clicked, this, &httppost::custom_open_file);


    connect(ui.radioButtonCustomFiles, &QRadioButton::clicked, this, &httppost::custom_type_button_clicked);
    connect(ui.radioButtonCustomTexts, &QRadioButton::clicked, this, &httppost::custom_type_button_clicked);

    connect(ui.pushButtonHeadersUp, &QPushButton::clicked, ui.tableWidgetHeaders, &table_widget::move_selected_row_up);
    connect(ui.pushButtonHeadersDown, &QPushButton::clicked, ui.tableWidgetHeaders, &table_widget::move_selected_row_down);
    connect(ui.pushButtonQueryUp, &QPushButton::clicked, ui.tableWidgetQuery, &table_widget::move_selected_row_up);
    connect(ui.pushButtonQueryDown, &QPushButton::clicked, ui.tableWidgetQuery, &table_widget::move_selected_row_down);
    connect(ui.pushButtonUrlEncodeUp, &QPushButton::clicked, ui.tableWidgetUrlEncode, &table_widget::move_selected_row_up);
    connect(ui.pushButtonUrlEncodeDown, &QPushButton::clicked, ui.tableWidgetUrlEncode, &table_widget::move_selected_row_down);
    connect(ui.pushButtonFormDataUp, &QPushButton::clicked, ui.tableWidgetFormData, &table_widget::move_selected_row_up);
    connect(ui.pushButtonFormDataDown, &QPushButton::clicked, ui.tableWidgetFormData, &table_widget::move_selected_row_down);

    connect(ui.pushButtonInsert, &QPushButton::clicked, ui.tableWidgetUrlEncode, &table_widget::insert);
    connect(ui.pushButtonRemove, &QPushButton::clicked, ui.tableWidgetUrlEncode, &table_widget::remove_selected);

	connect(ui.pushButtonFormInsertText, &QPushButton::clicked, this, &httppost::form_add_text);
	connect(ui.pushButtonFormInsertImage, &QPushButton::clicked, this, &httppost::form_add_image);
	connect(ui.pushButtonFormInsertRaw, &QPushButton::clicked, this, &httppost::form_add_raw);

    connect(ui.pushButtonFormInsert, &QPushButton::clicked, ui.tableWidgetFormData, &table_widget::insert);
    connect(ui.pushButtonFormRemove, &QPushButton::clicked, ui.tableWidgetFormData, &table_widget::remove_selected);

    connect(ui.pushButtonHeaderInsert, &QPushButton::clicked, ui.tableWidgetHeaders, &table_widget::insert);
    connect(ui.pushButtonHeaderRemove, &QPushButton::clicked, ui.tableWidgetHeaders, &table_widget::remove_selected);

    connect(ui.pushButtonQueryInsert, &QPushButton::clicked, ui.tableWidgetQuery, &table_widget::insert);
    connect(ui.pushButtonQueryRemove, &QPushButton::clicked, ui.tableWidgetQuery, &table_widget::remove_selected);

	set_table_widgets();

	//
    ui.tabWidgetPostBody->tabBar()->hide();
    ui.checkBoxAutoUCenterAuthentication->hide();

    ui.tabWidgetCustom->tabBar()->hide();

	progress_bar = new QProgressBar;
	progress_bar->setVisible(false);
	progress_bar->setMaximum(0);
	ui.statusBar->addPermanentWidget(progress_bar);

	ui.tabWidgetPostBody->setCurrentIndex(0);
    ui.statusBar->showMessage("Jadder Http 测试工具 V 0.5.0 build 2016.3.5 e-mail: JadderBao@163.com");

    load_form_status();

    QString default_file_name = DEFAULT_FILE_NAME;
    QJsonObject v = load_json_file(default_file_name);
    set_ui_data(v);


}

httppost::~httppost()
{
    QJsonObject v = get_ui_data();
    save_json_file(DEFAULT_FILE_NAME, v);

    save_form_status();
}

QString httppost::get_combobox_items(QComboBox *comboBox)
{
    QString content_types;
    for(int i=0; i< comboBox->count(); i++){
        content_types.append(comboBox->itemText(i));
        if(i < comboBox->count() - 1){
            content_types.append(',');
        }
    }

    return content_types;
}

void httppost::save_form_status()
{
    QJsonObject v;

    v["tab_widget_index"] = ui.tabWidget->currentIndex();
    v["custom_type"] = ui.radioButtonCustomFiles->isChecked() ? 1 : 2;
    v["custom_content_type"] =  get_combobox_items(ui.comboBoxCustomContentTypes);
    v["custom_files"] = get_combobox_items(ui.comboBoxCustomFiles);

    save_json_file(FORM_STATUS_FILE_NAME, v);
}

void httppost::update_custom_type_ui(int custom_type)
{
    if(custom_type == 1){
        ui.radioButtonCustomFiles->setChecked(true);
        ui.tabWidgetCustom->setCurrentWidget(ui.tabCustomFiles);
    } else if(custom_type == 2) {
        ui.radioButtonCustomTexts->setChecked(true);
        ui.tabWidgetCustom->setCurrentWidget(ui.tabCustomTexts);
    }
}

void httppost::load_form_status()
{
    QJsonObject v = load_json_file(FORM_STATUS_FILE_NAME);
    if(v.isEmpty()){
        return;
    }

    ui.tabWidget->setCurrentIndex(v["tab_widget_index"].toInt());

    int custom_type = v["custom_type"].toInt();
    update_custom_type_ui(custom_type);

    QStringList custom_content_types = v["custom_content_type"].toString().split(',');
    ui.comboBoxCustomContentTypes->clear();
    ui.comboBoxCustomContentTypes->addItems(custom_content_types);

    QStringList custom_files = v["custom_files"].toString().split(",");
    ui.comboBoxCustomFiles->clear();
    ui.comboBoxCustomFiles->addItems(custom_files);


}

void httppost::set_table_widgets()
{
    ui.tableWidgetHeaders->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui.tableWidgetHeaders->set_post_type(http_data::POST_TYPE_HEADER);
    ui.tableWidgetQuery->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui.tableWidgetQuery->set_post_type(http_data::POST_TYPE_URL_QUERY);
    ui.tableWidgetUrlEncode->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui.tableWidgetUrlEncode->set_post_type(http_data::POST_TYPE_BODY);
	ui.tableWidgetFormData->setEditTriggers(QAbstractItemView::AllEditTriggers);

	//设置Value列宽度
	ui.tableWidgetResponseHeaders->setColumnWidth(1, 600);

	table_widget_delegate::column_data_map_t map;

    table_widget_delegate::column_data_ptr used_column_data(table_widget_delegate::column_data_ptr::create());
    used_column_data->type = table_widget_delegate::WIDGET_CHECKBOX;

    map.insert(key_value_table_widget::KEY_VALUE_COLUMN_USED, used_column_data);

	table_widget_delegate * headers_delgate = new table_widget_delegate(map, ui.tableWidgetHeaders);
	ui.tableWidgetHeaders->setItemDelegate(headers_delgate);


    table_widget_delegate * query_delgate = new table_widget_delegate(map, ui.tableWidgetQuery);
    ui.tableWidgetQuery->setItemDelegate(query_delgate);

	table_widget_delegate * url_encode_delgate = new table_widget_delegate(map, ui.tableWidgetUrlEncode);
	ui.tableWidgetUrlEncode->setItemDelegate(url_encode_delgate);


    //form data table widget
    table_widget_delegate::column_data_map_t form_data_map;
    form_data_map.insert(form_data_table_widget::FORM_DATA_COLUMN_USED, used_column_data);

    table_widget_delegate::column_data_ptr content_type_column_data(table_widget_delegate::column_data_ptr::create());
    content_type_column_data->type = table_widget_delegate::WIDGET_COMBOBOX;
    content_type_column_data->data = QStringList() << POST_VALUE_TYPE_VALUE_STR << POST_VALUE_TYPE_UCENTER_AUTHCODE_STR
        << POST_VALUE_TYPE_JAVASCRIPT_STR;

    table_widget_delegate * form_data_delgate = new table_widget_delegate(form_data_map, ui.tableWidgetFormData);
    ui.tableWidgetFormData->setItemDelegate(form_data_delgate);

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

void httppost::content_type_button_clicked(bool checked)
{
    Q_UNUSED(checked)

    QRadioButton *button = qobject_cast<QRadioButton *>(sender());
    if(!button){
        return;
    }

    if(ui.radioButtonUrlEncoded->isChecked()){
        ui.tabWidgetPostBody->setCurrentWidget(ui.tabWidgetPostBodyPageUrlEncode);
    }else if(ui.radioButtonFormData->isChecked()){
        ui.tabWidgetPostBody->setCurrentWidget(ui.tabWidgetPostBodyPageFormData);
    }else if(ui.radioButtonJson->isChecked()){
        ui.tabWidgetPostBody->setCurrentWidget(ui.tabWidgetPostBodyPageJson);
    }else if(ui.radioButtonCustom->isChecked()){
        ui.tabWidgetPostBody->setCurrentWidget(ui.tabWidgetPostBodyPageCustom);
    }

    ui.comboBoxCustomContentTypes->setEnabled(ui.radioButtonCustom->isChecked());

}

void httppost::custom_type_button_clicked(bool checked)
{
    Q_UNUSED(checked)

    if(ui.radioButtonCustomFiles->isChecked()){
       ui.tabWidgetCustom->setCurrentWidget(ui.tabCustomFiles);
    }else if(ui.radioButtonCustomTexts->isChecked()){
        ui.tabWidgetCustom->setCurrentWidget(ui.tabCustomTexts);
    }
}

void httppost::custom_open_file()
{
    QString file_name = get_open_file_name("*.*(*.*)");
    if(file_name.isEmpty()){
        return;
    }

    ui.comboBoxCustomFiles->setCurrentText(file_name);
}

QUrl httppost::get_url()
{
    QUrl url(ui.lineEditUrl->text());
    if (!url.isValid()){
        QMessageBox::warning(this, "Url", "无效的Url", QMessageBox::Ok);
        ui.lineEditUrl->setFocus();
        return QUrl();
    }

    return url;
}

bool httppost::update_request_body(http_request *request)
{
    http_request_body *body = 0;
    QString content_type;
    switch (ui.tabWidgetPostBody->currentIndex()){
    case 0: // url_encoded
        {
            http_data_list url_encoded_items = ui.tableWidgetUrlEncode->data();
            body = new http_request_body(url_encoded_items.to_url_encode_string().toUtf8());
            content_type =  "application/x-www-form-urlencoded";
        }
        break;
    case 1:
        {
            QHttpMultiPart *multi_part = new QHttpMultiPart();
            form_data_table_to_multi_part(multi_part, ui.tableWidgetFormData);
            body = new http_request_body(multi_part);
        }
        break;
    case 2: //json
        body = new http_request_body(ui.plainTextEditJson->toPlainText().toUtf8());
        content_type =  "application/json";
        break;
    case 3: //custom
        if(ui.radioButtonCustomFiles->isChecked()){
            QString file_name = ui.comboBoxCustomFiles->currentText();
            QFile *file = new QFile(file_name);
            if(!file->open(QIODevice::ReadOnly)){
                QMessageBox::critical(this, "文件打开错误", file->errorString(), QMessageBox::Ok);
                delete file;
                return false;
            }

            if(ui.comboBoxCustomFiles->findText(file_name) == -1){
                ui.comboBoxCustomFiles->addItem(file_name);
            }

            body = new http_request_body(file);
        }else{
            body = new http_request_body(ui.plainTextEditCustomText->toPlainText().toUtf8());
        }
        content_type =  ui.comboBoxCustomContentTypes->currentText();
        if(ui.comboBoxCustomContentTypes->findText(content_type) == -1){
            ui.comboBoxCustomContentTypes->addItem(content_type);
        }
        break;
    default:
        break;
    }

    request->set_body(body);
    if(!content_type.isEmpty()){
        request->request()->setHeader(QNetworkRequest::ContentTypeHeader, content_type);
    }

    return true;
}

void httppost::send()
{
    QUrl url = get_url();
    if (!url.isValid()){
        return;
    }

    http_data_list query_items = ui.tableWidgetQuery->data();
    //原来 url, query 上的参数都一起发送
    QString url_query = query_items.to_url_encode_string(http_data::POST_TYPE_URL_QUERY);
    url.setQuery(url.hasQuery() ? url.query() + "&" + url_query : url_query);

    QSharedPointer<http_client> client(new http_client(am));
    connect(client.data(), &http_client::downloadProgress, this, &httppost::downloadProgress);
    connect(client.data(), &http_client::uploadProgress, this, &httppost::uploadProgress);
    connect(client.data(), &http_client::finished, this, &httppost::finished);

    //处理自定义头
    QSharedPointer<http_request> request(new http_request(url));
    http_data_list header_items = ui.tableWidgetHeaders->data();
    update_request_custom_header(header_items, request->request());

    QNetworkReply *reply = 0;
    QString verb = ui.comboBoxVerb->currentText();
    QString verb_upper = verb.toUpper();
    if(verb_upper == "GET"){
        reply = client->get(request.data());
    }else if(verb == "POST"){
        if(!update_request_body(request.data())){
            return;
        }
        reply = client->post(request.data());
    }else if(verb == "HEADER"){
        reply = client->customRequest(request.data(), verb);
    }else{
        if(!update_request_body(request.data())){
            return;
        }
        reply = client->customRequest(request.data(), verb);
    }

    if(reply){
        show_reply(reply);
    }
}

void httppost::set_ui_data(QJsonObject &v)
{
    ui.comboBoxVerb->setCurrentText(v["verb"].toString());
    ui.lineEditUrl->setText(v["url"].toString());
    ui.tableWidgetHeaders->set_data(http_data_list::from_json(v["headers"]));
    ui.tableWidgetQuery->set_data(http_data_list::from_json(v["query"]));

    QJsonObject body = v["body"].toObject();
    ui.tableWidgetUrlEncode->set_data(http_data_list::from_json(body["form_urlencoded"]));
    ui.tableWidgetFormData->set_data(http_form_data_list::from_json(body["multipart_form_data"]));
    ui.plainTextEditJson->setPlainText(body["application_json"].toString());


    ui.comboBoxCustomFiles->setCurrentText(body["custom_file"].toString());
    ui.plainTextEditCustomText->setPlainText(body["custom_text"].toString());
    ui.comboBoxCustomContentTypes->setCurrentText(body["custom_content_type"].toString());
    int custom_type = body["custom_type"].toInt();
    update_custom_type_ui(custom_type);

    int body_page_index = body["body_content_index"].toInt();
    ui.tabWidgetPostBody->setCurrentIndex(body_page_index);

    const int content_radio_button_size = 4;
    QRadioButton* content_radio_buttons[content_radio_button_size] = {ui.radioButtonUrlEncoded, ui.radioButtonFormData
                                           , ui.radioButtonJson, ui.radioButtonCustom};
    if(body_page_index >= 0 && body_page_index < 4){
        content_radio_buttons[body_page_index]->setChecked(true);
    }

}

QJsonObject httppost::get_ui_data()
{
    //table 转json 格式
    QJsonObject v;
    v["json_file_version"] = JSON_FILE_VERSION;
    v["verb"] = ui.comboBoxVerb->currentText();
    v["url"] = ui.lineEditUrl->text();
    v["headers"] = ui.tableWidgetHeaders->data().to_json();
    v["query"] = ui.tableWidgetQuery->data().to_json();

    QJsonObject body;
    body["body_content_index"] = ui.tabWidgetPostBody->currentIndex();
    body["form_urlencoded"] =ui.tableWidgetUrlEncode->data().to_json();
    body["multipart_form_data"] =ui.tableWidgetFormData->data().to_json();
    body["application_json"] =ui.plainTextEditJson->toPlainText();

    body["custom_type"] = ui.radioButtonCustomFiles->isChecked() ? 1 : 2; //1. file  2. text
    body["custom_text"] =ui.plainTextEditCustomText->toPlainText();
    body["custom_file"] =ui.comboBoxCustomFiles->currentText();
    body["custom_content_type"] = ui.comboBoxCustomContentTypes->currentText();

    v["body"] = body;

    return v;
}

void httppost::load()
{
    QString file_name = get_open_file_name("Http Post Data(*.hpd)");
    if(file_name.isEmpty()){
        return;
    }

    QJsonObject  v = load_json_file(file_name);
    set_ui_data(v);
}

void httppost::save()
{
    QString file_name = get_save_file_name("Http Post Data(*.hpd)");
    if(file_name.isEmpty()){
        return;
    }

    QJsonObject v = get_ui_data();
    save_json_file(file_name, v);
}

void httppost::save_as()
{

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
    items.append(ui.tableWidgetQuery->data());
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

void httppost::update_body_to_multi_part(const http_data_list &items, QHttpMultiPart * multi_part)
{
	//更新需要提交到body上的值
	for (int i = 0; i < items.count(); i++){
		const http_data_ptr& hdp = items[i];
		if (hdp->post_type() == http_data::POST_TYPE_BODY){
			QHttpPart textPart;
			textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
				QVariant(QString("form-data; name=\"%1\"").arg(hdp->key())));
			textPart.setBody(hdp->value().toUtf8());
			multi_part->append(textPart);
		}
	}
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

void httppost::form_data_table_to_multi_part(QHttpMultiPart* multi_part,
	QTableWidget *tableWidget)
{
	for (int row = 0; row < tableWidget->rowCount(); row++){

		QTableWidgetItem *name_item = tableWidget->item(row, form_data_table_widget::FORM_DATA_COLUMN_NAME);
		QString name = name_item ? name_item->data(0).toString() : "";

        QTableWidgetItem *content_type_item = tableWidget->item(row, form_data_table_widget::FORM_DATA_COLUMN_USED);
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
				QVariant(QString("form-data; name=\"%1\"; filename=\"%2\"").arg(name).arg(file->fileName())));
			part.setBodyDevice(file);
			multi_part->append(part);
		}

	}

	multi_part->setContentType(QHttpMultiPart::FormDataType);
}

void httppost::update_request_custom_header(const http_data_list& items, QNetworkRequest *request)
{
	for (auto it = items.begin(); it != items.end(); ++it){
		http_data_constptr pdi = *it;
		QString key = pdi->key();
        if (!pdi->used() || key.isEmpty()){
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
    ui.tableWidgetFormData->setItem(row, form_data_table_widget::FORM_DATA_COLUMN_USED,
        new QTableWidgetItem("true"));
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

