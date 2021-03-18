#include <iostream>
#include <Serialization/Json.h>

Grafkit::Json Grafkit::Serializer::JsonAdapter::ParseJson(IStream & stream)
{
	if (!stream) { throw std::runtime_error("Invalid stream"); }
	StreamData outBuffer{};
	if (!stream.ReadAll(outBuffer)) { throw std::runtime_error("Cannot read stream"); }
	return Json::parse(outBuffer); // TODO: stream can be passed directly as well
}

void Grafkit::Serializer::JsonAdapter::DumpJson(IStream & stream, const Json & json)
{
	if (!stream) { throw std::runtime_error("Invalid stream"); }
	const auto jsonData = json.dump();
	stream.Write(jsonData.c_str(), jsonData.size());
}
