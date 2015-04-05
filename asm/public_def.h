#include"asmlog.h"

typedef unsigned long long INT64;

#define ASM_MAXNUM 100
#define VREP_SERVER 6621

#define ASM_PORT 6608
#define ASM_IP "127.0.0.1"
#define AS_PORT 7778

#define RTSP_S7_REQUIRE "com.comcast.ngod.s7"
#define RTSP_R8_REQUIRE "com.comcast.ngod.r8"

#define RTSP_ID_S6_SETUP 0x0001
#define RTSP_ID_S6_TEARDOWN 0x0002
//#define VREP_ID_D6_OPEN 0x0003
//#define VREP_ID_D6_UPDATE 0x0004
//#define VREP_ID_D6_NOTIFICATION 0x0005

#define RTSP_METHOD_SETUP "SETUP"
#define RTSP_METHOD_TEARDOWN "TEARDOWN"
#define RTSP_METHOD_ANNOUNCE "ANNOUNCE"
#define RTSP_METHOD_GET_PARAMETERS "GET_PARAMETERS"

//D6消息类型
#define VREP_ID_D6_OPEN         0x01
#define VREP_ID_D6_UPDATE       0x02
#define VREP_ID_D6_NOTIFICATION 0x03
#define VREP_ID_D6_KEEPALIVE    0x04

//D6 OPEN参数类型
#define VREP_ID_D6_OPEN_CapabilityInformation    1
#define VREP_ID_D6_OPEN_StreamingZoneName        2
#define VREP_ID_D6_OPEN_ComponentName            3
#define VREP_ID_D6_OPEN_VendorSpecificString     4

#define VREP_ID_D6_OPEN_CapabilityInformation_RouteTypesSupported      1
#define VREP_ID_D6_OPEN_CapabilityInformation_SendReceiveCapability    2

//D6 UPDATE参数类型
#define VREP_ID_D6_UPDATE_WithdrawnRoutes          0x01
#define VREP_ID_D6_UPDATE_ReachableRoutes          0x02
#define VREP_ID_D6_UPDATE_NextHopServer            0x03
#define VREP_ID_D6_UPDATE_QAMNames                 0xE8
#define VREP_ID_D6_UPDATE_TotalBandwidth           0xEA
#define VREP_ID_D6_UPDATE_AvailableBandwidth       0xEB
#define VREP_ID_D6_UPDATE_Cost                     0xEC
#define VREP_ID_D6_UPDATE_EdgeInput                0xED
#define VREP_ID_D6_UPDATE_QAMParameters            0xEE
#define VREP_ID_D6_UPDATE_UDPMap                   0xEF
#define VREP_ID_D6_UPDATE_ServiceStatus            0xF1      
#define VREP_ID_D6_UPDATE_MaxMpegFlows             0xF2
#define VREP_ID_D6_UPDATE_NextHopServerAlternates  0xF3
#define VREP_ID_D6_UPDATE_OutputPort               0xF4

//D6 NOTIFICATION错误代码
#define VREP_ID_D6_NOTIFICATION_MessageHeaderError         1 
#define VREP_ID_D6_NOTIFICATION_OPENMessageError           2
#define VREP_ID_D6_NOTIFICATION_UPDATEMessageError         3
#define VREP_ID_D6_NOTIFICATION_HoldTimerExpired           4
#define VREP_ID_D6_NOTIFICATION_FiniteStateMachineError    5
#define VREP_ID_D6_NOTIFICATION_Cease                      6

   //Message Header Error Subcodes:
#define VREP_ID_D6_NOTIFICATION_MessageHeaderError_BadMessageLength        1
#define VREP_ID_D6_NOTIFICATION_MessageHeaderError_BadMessageType          2
   //OPEN Message Error Subcodes:
#define VREP_ID_D6_NOTIFICATION_OPENMessageError_UnsupportedVersionNumber          1
#define VREP_ID_D6_NOTIFICATION_OPENMessageError_BadPeerAddressDomain              2
#define VREP_ID_D6_NOTIFICATION_OPENMessageError_BadVREPIdentifier                 3
#define VREP_ID_D6_NOTIFICATION_OPENMessageError_UnsupportedOptionalParameter      4
#define VREP_ID_D6_NOTIFICATION_OPENMessageError_UnacceptableHoldTime              5
#define VREP_ID_D6_NOTIFICATION_OPENMessageError_UnsupportedCapability             6
#define VREP_ID_D6_NOTIFICATION_OPENMessageError_CapabilityMismatch                7
   //UPDATE Message Error Subcodes:
#define VREP_ID_D6_NOTIFICATION_UPDATEMessageError_MalformedAttributeList                1
#define VREP_ID_D6_NOTIFICATION_UPDATEMessageError_nrecognizedWellknownAttribute         2
#define VREP_ID_D6_NOTIFICATION_UPDATEMessageError_MissingWellknownMandatoryAttribute    3
#define VREP_ID_D6_NOTIFICATION_UPDATEMessageError_AttributeFlagsError                   4
#define VREP_ID_D6_NOTIFICATION_UPDATEMessageError_AttributeLengthError                  5
#define VREP_ID_D6_NOTIFICATION_UPDATEMessageError_InvalidAttribute                      6
#define RTSP_VERSION "RTSP/1.0" //RTSP协议版本
#define SESSION_GRUOP "SM1"  //会话组（每个SM一个组），主要用来getpramater时获取session组的信息

#define RTSP_S1_REQUIRE "com.comcast.ngod.s1" 
#define RTSP_S3_REQUIRE "com.comcast.ngod.s3"
#define RTSP_S6_REQUIRE "com.comcast.ngod.s6"
#define RTSP_S7_REQUIRE "com.comcast.ngod.s7"

#define QAM_NUM_MAX 5  //定义QAM信息的最大个数

#define RTSP_ID_S1_SETUP      1
#define RTSP_ID_S1_TEARDOWN   2

#define RTSP_METHOD_SETUP "SETUP"
#define RTSP_METHOD_TEARDOWN "TEARDOWN"
#define RTSP_METHOD_ANNOUNCE "ANNOUNCE"
#define RTSP_METHOD_GET_PARAMETERS "GET_PARAMETERS"
#define RTSP_METHOD_PING "PING"

//DEFINE RTSP HEADER
#define RTSP_HEADER_CSEQ "CSeq"
#define RTSP_HEADER_SESSION "Session"
#define RTSP_HEADER_REQUIRE "Require"
#define RTSP_HEADER_REASON "Reason"
#define RTSP_HEADER_NOTICE "Notice"
#define RTSP_HEADER_SESSION_GUOUP "SessionGroup"
#define RTSP_HEADER_TRANSPORT "Transport"
#define RTSP_HEADER_ONDEMANDSESSIONID "OnDemandSessionId"
#define RTSP_HEADER_POLICY "Policy"
#define RTSP_HEADER_EMBEDDED_ENCRYPTOR "EmbeddedEncryptor"
#define RTSP_HEADER_CLIENTSESSIONID "ClientSessionId"
#define RTSP_HEADER_CONTENT_TYPE "Content-type"
#define RTSP_HEADER_CONTENT_LENGTH "Content-Length"
#define RTSP_HEADER_STOP_POINT "StopPoint"

//#define RTSP_TEARDOWN_REASON_CODE
#define  RTSP_TEARDOWN_REASON_CODE_User_stop		200
#define  RTSP_TEARDOWN_REASON_CODE_End_of_stream		201
#define  RTSP_TEARDOWN_REASON_CODE_Beginning_of_stream		202
#define  RTSP_TEARDOWN_REASON_CODE_Pause_timeout		203
#define  RTSP_TEARDOWN_REASON_CODE_Fail_to_tune			400
#define  RTSP_TEARDOWN_REASON_CODE_loss_of_tune			401
#define  RTSP_TEARDOWN_REASON_CODE_Loss_of_tune			402
#define  RTSP_TEARDOWN_REASON_CODE_RTSP_failure			403
#define  RTSP_TEARDOWN_REASON_CODE_Channel_failure			404
#define  RTSP_TEARDOWN_REASON_CODE_No_RTSP_server			405
#define  RTSP_TEARDOWN_REASON_CODE_Trick_play_failed		406
#define  RTSP_TEARDOWN_REASON_CODE_Internal_ODA_issue		407
#define  RTSP_TEARDOWN_REASON_CODE_Unknown			        408
#define  RTSP_TEARDOWN_REASON_CODE_Network_Resource_Failure			409
#define  RTSP_TEARDOWN_REASON_CODE_Settop_Heartbeat_Timeout			420
#define  RTSP_TEARDOWN_REASON_CODE_Settop_Inactivity_Timeout		421
#define  RTSP_TEARDOWN_REASON_CODE_Content_Unavailable			422
#define  RTSP_TEARDOWN_REASON_CODE_Streaming_Failure			423
#define  RTSP_TEARDOWN_REASON_CODE_QAM_Failure			424
#define  RTSP_TEARDOWN_REASON_CODE_Volume_Failure		425
#define  RTSP_TEARDOWN_REASON_CODE_Stream_Control_Error			426
#define  RTSP_TEARDOWN_REASON_CODE_Stream_Control_Timeout		427
#define  RTSP_TEARDOWN_REASON_CODE_Session_List_Mismatch		428
#define  RTSP_TEARDOWN_REASON_CODE_Session_timeout			550


//RTSP response codes
#define RTSP_ResponseCode_OK                                       200 //OK
#define RTSP_ResponseCode_BadRequest                          400 //N/A
#define RTSP_ResponseCode_Forbidden                            403 //N/A
#define RTSP_ResponseCode_NotFound                            404 //TSID not found; TSID does not exist on this Edge QAM, or invalid URL
#define RTSP_ResponseCode_MethodNotAllowed             405 //Method Not Allowed. Example = Play
#define RTSP_ResponseCode_NotAcceptable                     406 //N/A
#define RTSP_ResponseCode_RequestTimeOut                  408 //Request Time Out
#define RTSP_ResponseCode_Gone                                    410 //N/A
#define RTSP_ResponseCode_RequestEntityTooLarge         413 //N/A
#define RTSP_ResponseCode_UnsupportedMediaType	      415 //N/A
#define RTSP_ResponseCode_InvalidParameter		      451 //Invalid Parameter
#define RTSP_ResponseCode_NotEnoughBandwidth          453 //QAM Bandwidth Exceeded
#define RTSP_ResponseCode_SessionNotFound                 454 //Session Not Found
#define RTSP_ResponseCode_InvalidRange                        457 //Invalid Range
#define RTSP_ResponseCode_AggregateOperationNotAllowed                              459 //N/A
#define RTSP_ResponseCode_UnsupportedTransport                                             461 //Unsupported Transport
#define RTSP_ResponseCode_DestinationUnreachable                                           462 //Invalid Destination IP Address in Transport specification.
#define RTSP_ResponseCode_GatewayTimeout                                                      504 //N/A
#define RTSP_ResponseCode_RTSPVersionNotSupported                                       505 //RTSP Version Not Supported
//SM
#define RTSP_ResponseCode_SMSetupFailed_UnknownQAMGroup	                    651//SM unable to find specified QAM group
#define RTSP_ResponseCode_SMSetupFailed_InvalidRequest	                    652//Request sent to SM was invalid
#define RTSP_ResponseCode_SMSetupFailed_InternalError	                    653//Vendor-specific error occurred on SM.
//PS(Purchase Server)
#define RTSP_ResponseCode_PSSetupFailed_NoResponse	                        660//No response received from Purchase Server
#define RTSP_ResponseCode_PSSetupFailed_UnknownPurchaseToken	            661//Purchase server did not recognize purchase token
#define RTSP_ResponseCode_PSSetupFailed_InvalidRequest                      662//Purchase server failed to parse request sent to it.
#define RTSP_ResponseCode_PSSetupFailed_InternalError	                    663//Vendor-specific error occurred on Purchase Server
//ERM
#define RTSP_ResponseCode_ERMSetupFailed_NoResponse	                        670//No response received from ERM
#define RTSP_ResponseCode_ERMSetupFailed_InvalidRequest                     671//The ERM failed to parse the request from the SM
#define RTSP_ResponseCode_ERMSetupFailed_QAMBandwidthNotAvailable           672//No bandwidth was available on any suitable QAM.
#define RTSP_ResponseCode_ERMSetupFailed_NetworkBandwidthNotAvailable 	    673//No suitable network bandwidth was available to perform this request.
#define RTSP_ResponseCode_ERMSetupFailed_ProgramNotAvailable			    674//No suitable program was available to perform this request.
#define RTSP_ResponseCode_ERMSetupFailed_ServiceGroupNotFound               675//The service group specified in the request was not found.
#define RTSP_ResponseCode_ERMSetupFailed_QAMGroupsNotFound 		            676//The QAM group(s) specified in the request was/were not found.
#define RTSP_ResponseCode_ERMSetupFailed_QAMNotAvailable			        677//No suitable QAM was available to carry out the specified request.
#define RTSP_ResponseCode_ERMSetupFailed_EdgeDeviceNotAvailable             678//No suitable Edge Device was available to carry out the specified request.
#define RTSP_ResponseCode_ERMSetupFailed_InternalError	                    679//Vendor-specific error occurred on ERM.
//APM
#define RTSP_ResponseCode_APMLocateAssetFailed_NoResponse	                690//No response received from APM
#define RTSP_ResponseCode_APMLocateAssetFailed_AssetNotAvailable	        691//APM reported that requested asset was not available
#define RTSP_ResponseCode_APMLocateAssetFailed_InvalidRequest	            692//APM was unable to parse request from ODRM
#define RTSP_ResponseCode_APMLocateAssetFailed_InternalError	            693//Vendor-specific error occurred on APM
//ODRM
#define RTSP_ResponseCode_ODRMSetupFailed_NoResponse	                    750//No response received from ODRM
#define RTSP_ResponseCode_ODRMSetupFailed_UnknownSOPGroup	                751//Specified SOP group is unknown to ODRM
#define RTSP_ResponseCode_ODRMSetupFailed_BandwidthNotAvailable	            752//No suitable SOP Bandwidth available
#define RTSP_ResponseCode_ODRMSetupFailed_StreamNotAvailable	            753//No streams available on any suitable server
#define RTSP_ResponseCode_ODRMSetupFailed_AssetNotAvailable	                754//Unable to locate specified Asset
#define RTSP_ResponseCode_ODRMSetupFailed_InvalidRequest	                755//Request received by ODRM was invalid
#define RTSP_ResponseCode_ODRMSetupFailed_InternalError	                    756//Vendor-specific error occurred on ODRM
//Suitable Server
#define RTSP_ResponseCode_ServerSetupFailed_NoResponse	                    770//No response received from suitable server
#define RTSP_ResponseCode_ServerSetupFailed_AssetNotFound                   771//Server unable to see specified Asset
#define RTSP_ResponseCode_ServerSetupFailed_SOPNotAvailable                 772//Suitable SOP unavailable to server
#define RTSP_ResponseCode_ServerSetupFailed_UnknownSOPGroup	                773//Specified SOP Group unknown to server
#define RTSP_ResponseCode_ServerSetupFailed_UnknownSOPNames	                774//Specified SOP Name(s) unknown to server
#define RTSP_ResponseCode_ServerSetupFailed_InsufficientVolumeBandwidth	    775//No suitable server with sufficient bandwidth available to suitable volume
#define RTSP_ResponseCode_ServerSetupFailed_InsufficientNetworkBandwidth	776//No suitable SOP with sufficient bandwidth to target QAM
#define RTSP_ResponseCode_ServerSetupFailed_InvalidRequest	                777//Server unable to parse request from ODRM
#define RTSP_ResponseCode_ServerSetupFailed_InternalError	                778//Vendor-Specific Error occurred on server

//ASM
#define RTSP_ResponseCode_ASMSetupFailed_NoResponse	                    780//No response received from ODRM
#define RTSP_ResponseCode_ASMSetupFailed_UnknownSOPGroup	                781//Specified SOP group is unknown to ODRM
#define RTSP_ResponseCode_ASMSetupFailed_BandwidthNotAvailable	            782//No suitable SOP Bandwidth available
#define RTSP_ResponseCode_ASMSetupFailed_StreamNotAvailable	            783//No streams available on any suitable server
#define RTSP_ResponseCode_ASMSetupFailed_AssetNotAvailable	                784//Unable to locate specified Asset
#define RTSP_ResponseCode_ASMSetupFailed_InvalidRequest	                785//Request received by ODRM was invalid
#define RTSP_ResponseCode_ASMSetupFailed_InternalError	                    786//Vendor-specific error occurred on ODRM



//RTSP Announce code
#define RTSP_ANNOUNCE_EndOfStreamReached				2101
#define RTSP_ANNOUNCE_StartOfStreamReached				2104									
#define RTSP_ANNOUNCE_ErrorReadingContentData				4400
#define RTSP_ANNOUNCE_ServerResourcesUnavailable				5200
#define RTSP_ANNOUNCE_DownstreamFailure				5401
#define RTSP_ANNOUNCE_ClientSessionTerminated				5402
#define RTSP_ANNOUNCE_InternalServerError				5502
#define RTSP_ANNOUNCE_InbandStreamMarkerMismatch				5601
#define RTSP_ANNOUNCE_BandwidthExceededLimit				5602//ngod文档中此值为5601，与上值相同。（5602为自定义值）
#define RTSP_ANNOUNCE_SessionInProgress				5700
#define RTSP_ANNOUNCE_EncryptionEngineFailure				6000
#define RTSP_ANNOUNCE_StreamBandwidthExceedsThatAvailable				6001
#define RTSP_ANNOUNCE_DownstreamDestinationUnreachable				6004
#define RTSP_ANNOUNCE_UnableToEncryptOneOrMoreComponents				6005
#define RTSP_ANNOUNCE_ECMGSessionFailure				6006

#define SS_PORT 7000
