// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: telemetry.proto

#include "telemetry.pb.h"
#include "telemetry.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>

static const char* TelemetryManager_method_names[] = {
  "/TelemetryManager/configure",
  "/TelemetryManager/publish",
  "/TelemetryManager/collect",
  "/TelemetryManager/enable",
  "/TelemetryManager/disable",
};

std::unique_ptr< TelemetryManager::Stub> TelemetryManager::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< TelemetryManager::Stub> stub(new TelemetryManager::Stub(channel, options));
  return stub;
}

TelemetryManager::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_configure_(TelemetryManager_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_publish_(TelemetryManager_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_collect_(TelemetryManager_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_enable_(TelemetryManager_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_disable_(TelemetryManager_method_names[4], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status TelemetryManager::Stub::configure(::grpc::ClientContext* context, const ::ConfigureMetadataRequest& request, ::ConfigureMetadataResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::ConfigureMetadataRequest, ::ConfigureMetadataResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_configure_, context, request, response);
}

void TelemetryManager::Stub::experimental_async::configure(::grpc::ClientContext* context, const ::ConfigureMetadataRequest* request, ::ConfigureMetadataResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::ConfigureMetadataRequest, ::ConfigureMetadataResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_configure_, context, request, response, std::move(f));
}

void TelemetryManager::Stub::experimental_async::configure(::grpc::ClientContext* context, const ::ConfigureMetadataRequest* request, ::ConfigureMetadataResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_configure_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::ConfigureMetadataResponse>* TelemetryManager::Stub::PrepareAsyncconfigureRaw(::grpc::ClientContext* context, const ::ConfigureMetadataRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::ConfigureMetadataResponse, ::ConfigureMetadataRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_configure_, context, request);
}

::grpc::ClientAsyncResponseReader< ::ConfigureMetadataResponse>* TelemetryManager::Stub::AsyncconfigureRaw(::grpc::ClientContext* context, const ::ConfigureMetadataRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncconfigureRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status TelemetryManager::Stub::publish(::grpc::ClientContext* context, const ::PublishRequest& request, ::PublishResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::PublishRequest, ::PublishResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_publish_, context, request, response);
}

void TelemetryManager::Stub::experimental_async::publish(::grpc::ClientContext* context, const ::PublishRequest* request, ::PublishResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::PublishRequest, ::PublishResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_publish_, context, request, response, std::move(f));
}

void TelemetryManager::Stub::experimental_async::publish(::grpc::ClientContext* context, const ::PublishRequest* request, ::PublishResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_publish_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::PublishResponse>* TelemetryManager::Stub::PrepareAsyncpublishRaw(::grpc::ClientContext* context, const ::PublishRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::PublishResponse, ::PublishRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_publish_, context, request);
}

::grpc::ClientAsyncResponseReader< ::PublishResponse>* TelemetryManager::Stub::AsyncpublishRaw(::grpc::ClientContext* context, const ::PublishRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncpublishRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status TelemetryManager::Stub::collect(::grpc::ClientContext* context, const ::CollectRequest& request, ::CollectResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::CollectRequest, ::CollectResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_collect_, context, request, response);
}

void TelemetryManager::Stub::experimental_async::collect(::grpc::ClientContext* context, const ::CollectRequest* request, ::CollectResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::CollectRequest, ::CollectResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_collect_, context, request, response, std::move(f));
}

void TelemetryManager::Stub::experimental_async::collect(::grpc::ClientContext* context, const ::CollectRequest* request, ::CollectResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_collect_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::CollectResponse>* TelemetryManager::Stub::PrepareAsynccollectRaw(::grpc::ClientContext* context, const ::CollectRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::CollectResponse, ::CollectRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_collect_, context, request);
}

::grpc::ClientAsyncResponseReader< ::CollectResponse>* TelemetryManager::Stub::AsynccollectRaw(::grpc::ClientContext* context, const ::CollectRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsynccollectRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status TelemetryManager::Stub::enable(::grpc::ClientContext* context, const ::EnableRequest& request, ::EnableResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::EnableRequest, ::EnableResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_enable_, context, request, response);
}

void TelemetryManager::Stub::experimental_async::enable(::grpc::ClientContext* context, const ::EnableRequest* request, ::EnableResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::EnableRequest, ::EnableResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_enable_, context, request, response, std::move(f));
}

void TelemetryManager::Stub::experimental_async::enable(::grpc::ClientContext* context, const ::EnableRequest* request, ::EnableResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_enable_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::EnableResponse>* TelemetryManager::Stub::PrepareAsyncenableRaw(::grpc::ClientContext* context, const ::EnableRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::EnableResponse, ::EnableRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_enable_, context, request);
}

::grpc::ClientAsyncResponseReader< ::EnableResponse>* TelemetryManager::Stub::AsyncenableRaw(::grpc::ClientContext* context, const ::EnableRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncenableRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status TelemetryManager::Stub::disable(::grpc::ClientContext* context, const ::DisableRequest& request, ::DisableResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::DisableRequest, ::DisableResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_disable_, context, request, response);
}

void TelemetryManager::Stub::experimental_async::disable(::grpc::ClientContext* context, const ::DisableRequest* request, ::DisableResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::DisableRequest, ::DisableResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_disable_, context, request, response, std::move(f));
}

void TelemetryManager::Stub::experimental_async::disable(::grpc::ClientContext* context, const ::DisableRequest* request, ::DisableResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_disable_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::DisableResponse>* TelemetryManager::Stub::PrepareAsyncdisableRaw(::grpc::ClientContext* context, const ::DisableRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::DisableResponse, ::DisableRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_disable_, context, request);
}

::grpc::ClientAsyncResponseReader< ::DisableResponse>* TelemetryManager::Stub::AsyncdisableRaw(::grpc::ClientContext* context, const ::DisableRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncdisableRaw(context, request, cq);
  result->StartCall();
  return result;
}

TelemetryManager::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      TelemetryManager_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< TelemetryManager::Service, ::ConfigureMetadataRequest, ::ConfigureMetadataResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](TelemetryManager::Service* service,
             ::grpc::ServerContext* ctx,
             const ::ConfigureMetadataRequest* req,
             ::ConfigureMetadataResponse* resp) {
               return service->configure(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      TelemetryManager_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< TelemetryManager::Service, ::PublishRequest, ::PublishResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](TelemetryManager::Service* service,
             ::grpc::ServerContext* ctx,
             const ::PublishRequest* req,
             ::PublishResponse* resp) {
               return service->publish(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      TelemetryManager_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< TelemetryManager::Service, ::CollectRequest, ::CollectResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](TelemetryManager::Service* service,
             ::grpc::ServerContext* ctx,
             const ::CollectRequest* req,
             ::CollectResponse* resp) {
               return service->collect(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      TelemetryManager_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< TelemetryManager::Service, ::EnableRequest, ::EnableResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](TelemetryManager::Service* service,
             ::grpc::ServerContext* ctx,
             const ::EnableRequest* req,
             ::EnableResponse* resp) {
               return service->enable(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      TelemetryManager_method_names[4],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< TelemetryManager::Service, ::DisableRequest, ::DisableResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](TelemetryManager::Service* service,
             ::grpc::ServerContext* ctx,
             const ::DisableRequest* req,
             ::DisableResponse* resp) {
               return service->disable(ctx, req, resp);
             }, this)));
}

TelemetryManager::Service::~Service() {
}

::grpc::Status TelemetryManager::Service::configure(::grpc::ServerContext* context, const ::ConfigureMetadataRequest* request, ::ConfigureMetadataResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status TelemetryManager::Service::publish(::grpc::ServerContext* context, const ::PublishRequest* request, ::PublishResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status TelemetryManager::Service::collect(::grpc::ServerContext* context, const ::CollectRequest* request, ::CollectResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status TelemetryManager::Service::enable(::grpc::ServerContext* context, const ::EnableRequest* request, ::EnableResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status TelemetryManager::Service::disable(::grpc::ServerContext* context, const ::DisableRequest* request, ::DisableResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

