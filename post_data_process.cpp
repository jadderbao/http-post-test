#include "post_data_process.h"
#include "http_script_engine.h"

post_data_process::post_data_process(http_script_engine *engine, QObject *parent)
	: QObject(parent)
	, _engine(engine)
{

}

post_data_process::~post_data_process()
{

}

bool post_data_process::process_post_data(http_data_list *datas)
{
	QScriptValue global_object = _engine->globalObject();
	for (auto it = datas->begin(); it != datas->end(); ++it){
		http_data_ptr pdp = *it;
		if (pdp->value_type() == POST_VALUE_TYPE_UNIX_TIMESTAMP_STR){
			pdp->set_value(_engine->evaluate(pdp->value()).toString());
		}
		else if (pdp->value_type() == POST_VALUE_TYPE_UCENTER_AUTHCODE_STR){
			http_data_list auth_list = datas->get_post_datas(http_data::POST_TYPE_ALL,
				http_data::HTTP_DATA_AUTH);
			code_tool du;
			QString value = du.ucenter_encode(pdp->value(), auth_list.to_url_encode_string());
			pdp->set_value(value);
		}
		else if (pdp->value_type() == POST_VALUE_TYPE_JAVASCRIPT_STR){
			pdp->set_value(_engine->evaluate(pdp->value()).toString());
		}
	}

	return true;
}

QString post_data_process::error_string()
{
	return _error_string;
}
