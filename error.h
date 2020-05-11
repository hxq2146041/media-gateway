#ifndef GATEWAY_ERROR_H
#define GATEWAY_ERROR_H

/*! \brief Success (no error) */
#define GATEWAY_OK								0

/*! \brief Unauthorized (can only happen when using apisecret/auth token) */
#define GATEWAY_ERROR_UNAUTHORIZED				403
/*! \brief Unauthorized access to a plugin (can only happen when using auth token) */
#define GATEWAY_ERROR_UNAUTHORIZED_PLUGIN			405
/*! \brief Unknown/undocumented error */
#define GATEWAY_ERROR_UNKNOWN						490
/*! \brief Transport related error */
#define GATEWAY_ERROR_TRANSPORT_SPECIFIC			450
/*! \brief The request is missing in the message */
#define GATEWAY_ERROR_MISSING_REQUEST				452
/*! \brief The Janus core does not suppurt this request */
#define GATEWAY_ERROR_UNKNOWN_REQUEST				453
/*! \brief The payload is not a valid JSON message */
#define GATEWAY_ERROR_INVALID_JSON				454
/*! \brief The object is not a valid JSON object as expected */
#define GATEWAY_ERROR_INVALID_JSON_OBJECT			455
/*! \brief A mandatory element is missing in the message */
#define GATEWAY_ERROR_MISSING_MANDATORY_ELEMENT	456
/*! \brief The request cannot be handled for this webserver path  */
#define GATEWAY_ERROR_INVALID_REQUEST_PATH		457
/*! \brief The session the request refers to doesn't exist */
#define GATEWAY_ERROR_SESSION_NOT_FOUND			458
/*! \brief The handle the request refers to doesn't exist */
#define GATEWAY_ERROR_HANDLE_NOT_FOUND			459
/*! \brief The plugin the request wants to talk to doesn't exist */
#define GATEWAY_ERROR_PLUGIN_NOT_FOUND			460
/*! \brief An error occurring when trying to attach to a plugin and create a handle  */
#define GATEWAY_ERROR_PLUGIN_ATTACH				461
/*! \brief An error occurring when trying to send a message/request to the plugin */
#define GATEWAY_ERROR_PLUGIN_MESSAGE				462
/*! \brief An error occurring when trying to detach from a plugin and destroy the related handle  */
#define GATEWAY_ERROR_PLUGIN_DETACH				463
/*! \brief The Janus core doesn't support this SDP type */
#define GATEWAY_ERROR_JSEP_UNKNOWN_TYPE			464
/*! \brief The Session Description provided by the peer is invalid */
#define GATEWAY_ERROR_JSEP_INVALID_SDP			465
/*! \brief The stream a trickle candidate for does not exist or is invalid */
#define GATEWAY_ERROR_TRICKE_INVALID_STREAM		466
/*! \brief A JSON element is of the wrong type (e.g., an integer instead of a string) */
#define GATEWAY_ERROR_INVALID_ELEMENT_TYPE		467
/*! \brief The ID provided to create a new session is already in use */
#define GATEWAY_ERROR_SESSION_CONFLICT			468
/*! \brief We got an ANSWER to an OFFER we never made */
#define GATEWAY_ERROR_UNEXPECTED_ANSWER			469
/*! \brief The auth token the request refers to doesn't exist */
#define GATEWAY_ERROR_TOKEN_NOT_FOUND				470
/*! \brief The current request cannot be handled because of not compatible WebRTC state */
#define GATEWAY_ERROR_WEBRTC_STATE				471
/*! \brief The server is currently configured not to accept new sessions */
#define GATEWAY_ERROR_NOT_ACCEPTING_SESSIONS		472
/*! \brief The plugin the request wants to talk to doesn't exist */
#define GATEWAY_ERROR_ASK_TYPE_NOT_FOUND			480
#define GATEWAY_ERROR_ASK_TYPE_EMPTY			481
#define GATEWAY_ERROR_ASK_TYPE_ERROR			482
#define GATEWAY_ERROR_ASK_PARAMS_ERROR			483
#define GATEWAY_ERROR_ASK_PEER_NOT_ONLINE			484

/*! \brief Helper method to get a string representation of an API error code
 * @param[in] error The API error code
 * @returns A string representation of the error code */
const char *gateway_api_error(int error);

#endif
