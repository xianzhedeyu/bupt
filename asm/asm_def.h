#include "asmlog.h"

typedef unsigned long long E_ERM_INT64;

#ifndef RTSP_METHOD_SETUP
#define RTSP_METHOD_SETUP "SETUP"
#define RTSP_METHOD_TEARDOWN "TEARDOWN"
#define RTSP_METHOD_ANNOUNCE "ANNOUNCE"
#define RTSP_METHOD_GET_PARAMETERS "GET_PARAMETERS"
#endif

#define DRRP_ID_ASM_OPEN         0x01
#define DRRP_ID_ASM_UPDATE       0x02
#define DRRP_ID_ASM_NOTIFICATION 0x03
#define DRRP_ID_ASM_KEEPALIVE    0x04

//ASM OPEN参数类型
#define DRRP_ID_ASM_OPEN_CapabilityInformation    1
#define DRRP_ID_ASM_OPEN_StreamingZoneName        2
#define DRRP_ID_ASM_OPEN_ComponentName            3
#define DRRP_ID_ASM_OPEN_VendorSpecificString     4

#define DRRP_ID_ASM_OPEN_CapabilityInformation_RouteTypesSupported      1
#define DRRP_ID_ASM_OPEN_CapabilityInformation_SendReceiveCapability    2

//ASM UPDATE参数类型
#define DRRP_ID_ASM_UPDATE_WithdrawnRoutes          0x01
#define DRRP_ID_ASM_UPDATE_ReachableRoutes          0x02
#define DRRP_ID_ASM_UPDATE_NextHopServer            0x03
#define DRRP_ID_ASM_UPDATE_QAMNames                 0xE8
#define DRRP_ID_ASM_UPDATE_TotalBandwidth           0xEA
#define DRRP_ID_ASM_UPDATE_AvailableBandwidth       0xEB
#define DRRP_ID_ASM_UPDATE_Cost                     0xEC
#define DRRP_ID_ASM_UPDATE_EdgeInput                0xED
#define DRRP_ID_ASM_UPDATE_QAMParameters            0xEE
#define DRRP_ID_ASM_UPDATE_UDPMap                   0xEF
#define DRRP_ID_ASM_UPDATE_ServiceStatus            0xF1      
#define DRRP_ID_ASM_UPDATE_MaxMpegFlows             0xF2
#define DRRP_ID_ASM_UPDATE_NextHopServerAlternates  0xF3
#define DRRP_ID_ASM_UPDATE_OutputPort               0xF4

//ASM NOTIFICATION错误代码
#define DRRP_ID_ASM_NOTIFICATION_MessageHeaderError         1 
#define DRRP_ID_ASM_NOTIFICATION_OPENMessageError           2
#define DRRP_ID_ASM_NOTIFICATION_UPDATEMessageError         3
#define DRRP_ID_ASM_NOTIFICATION_HoldTimerExpired           4
#define DRRP_ID_ASM_NOTIFICATION_FiniteStateMachineError    5
#define DRRP_ID_ASM_NOTIFICATION_Cease                      6

   //Message Header Error Subcodes:
#define DRRP_ID_ASM_NOTIFICATION_MessageHeaderError_BadMessageLength        1
#define DRRP_ID_ASM_NOTIFICATION_MessageHeaderError_BadMessageType          2
   //OPEN Message Error Subcodes:
#define DRRP_ID_ASM_NOTIFICATION_OPENMessageError_UnsupportedVersionNumber          1
#define DRRP_ID_ASM_NOTIFICATION_OPENMessageError_BadPeerAddressDomain              2
#define DRRP_ID_ASM_NOTIFICATION_OPENMessageError_BadDRRPIdentifier                 3
#define DRRP_ID_ASM_NOTIFICATION_OPENMessageError_UnsupportedOptionalParameter      4
#define DRRP_ID_ASM_NOTIFICATION_OPENMessageError_UnacceptableHoldTime              5
#define DRRP_ID_ASM_NOTIFICATION_OPENMessageError_UnsupportedCapability             6
#define DRRP_ID_ASM_NOTIFICATION_OPENMessageError_CapabilityMismatch                7
   //UPDATE Message Error Subcodes:
#define DRRP_ID_ASM_NOTIFICATION_UPDATEMessageError_MalformedAttributeList                1
#define DRRP_ID_ASM_NOTIFICATION_UPDATEMessageError_nrecognizedWellknownAttribute         2
#define DRRP_ID_ASM_NOTIFICATION_UPDATEMessageError_MissingWellknownMandatoryAttribute    3
#define DRRP_ID_ASM_NOTIFICATION_UPDATEMessageError_AttributeFlagsError                   4
#define DRRP_ID_ASM_NOTIFICATION_UPDATEMessageError_AttributeLengthError                  5
#define DRRP_ID_ASM_NOTIFICATION_UPDATEMessageError_InvalidAttribute                      6

//RTSP response codes applicable to interface ASM
#define RTSP_ASM_ResponseCode_OK                                       200 //OK
#define RTSP_ASM_ResponseCode_BadRequest                          400 //N/A
#define RTSP_ASM_ResponseCode_Forbidden                            403 //N/A
#define RTSP_ASM_ResponseCode_NotFound                            404 
#define RTSP_ASM_ResponseCode_MethodNotAllowed             405 //Method Not Allowed. Example = Play
#define RTSP_ASM_ResponseCode_NotAcceptable                     406 //N/A
#define RTSP_ASM_ResponseCode_RequestTimeOut                  408 //Request Time Out
#define RTSP_ASM_ResponseCode_Gone                                    410 //N/A
#define RTSP_ASM_ResponseCode_RequestEntityTooLarge         413 //N/A
#define RTSP_ASM_ResponseCode_UnsupportedMediaType	      415 //N/A
#define RTSP_ASM_ResponseCode_InvalidParameter		      451 //Invalid Parameter
#define RTSP_ASM_ResponseCode_NotEnoughBandwidth          453 //QAM Bandwidth Exceeded
#define RTSP_ASM_ResponseCode_SessionNotFound                 454 //Session Not Found
#define RTSP_ASM_ResponseCode_InvalidRange                        457 //Invalid Range
#define RTSP_ASM_ResponseCode_AggregateOperationNotAllowed                              459 //N/A
#define RTSP_ASM_ResponseCode_UnsupportedTransport                                             461 //Unsupported Transport
#define RTSP_ASM_ResponseCode_DestinationUnreachable                                           462 //Invalid Destination IP Address in Transport specification.
#define RTSP_ASM_ResponseCode_GatewayTimeout                                                      504 //N/A
#define RTSP_ASM_ResponseCode_RTSPVersionNotSupported                                       505 //RTSP Version Not Supported
#define RTSP_ASM_ResponseCode_ERMSetupFailed_InvalidRequest                                671 //The ERM failed to parse the request from the SM
#define RTSP_ASM_ResponseCode_ERMSetupFailed_QAMBandwidthNotAvailable           672 //No bandwidth was available on any suitable QAM.
#define RTSP_ASM_ResponseCode_ERMSetupFailed_NetworkBandwidthNotAvailable 	   673  //No suitable network bandwidth was available to perform this request.
#define RTSP_ASM_ResponseCode_ERMSetupFailed_ProgramNotAvailable			   674 //No suitable program was available to perform this request.
#define RTSP_ASM_ResponseCode_ERMSetupFailed_ServiceGroupNotFound                   675  //The service group specified in the request was not found.
#define RTSP_ASM_ResponseCode_ERMSetupFailed_QAMGroupsNotFound 		   676 //The QAM group(s) specified in the request was/were not found.
#define RTSP_ASM_ResponseCode_ERMSetupFailed_QAMNotAvailable			   677  //No suitable QAM was available to carry out the specified request.
#define RTSP_ASM_ResponseCode_ERMSetupFailed_EdgeDeviceNotAvailable                  678  //No suitable Edge Device was available to carry out the specified request.


