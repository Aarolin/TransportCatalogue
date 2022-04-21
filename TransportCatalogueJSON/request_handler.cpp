#include "request_handler.h"

using namespace reading_queries;

RequestHandler::RequestHandler(JSONRequestBuilder& request_responder)
	: request_responder_(request_responder) {

}


void RequestHandler::OutRequests(const json::Dict& map_requests, std::ostream& output) const {

	Print(request_responder_.MakeJSONResponseToRequest(map_requests), output);

}

