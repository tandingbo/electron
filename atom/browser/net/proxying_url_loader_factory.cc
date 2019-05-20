// Copyright (c) 2019 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/net/proxying_url_loader_factory.h"

namespace atom {

ProxyingURLLoaderFactory::ProxyingURLLoaderFactory(
    network::mojom::URLLoaderFactoryRequest loader_request,
    network::mojom::URLLoaderFactoryPtrInfo target_factory_info)
    : weak_factory_(this) {
  target_factory_.Bind(std::move(target_factory_info));
  target_factory_.set_connection_error_handler(base::BindOnce(
      &ProxyingURLLoaderFactory::OnTargetFactoryError, base::Unretained(this)));
  proxy_bindings_.AddBinding(this, std::move(loader_request));
  proxy_bindings_.set_connection_error_handler(base::BindRepeating(
      &ProxyingURLLoaderFactory::OnProxyBindingError, base::Unretained(this)));
}

ProxyingURLLoaderFactory::~ProxyingURLLoaderFactory() = default;

void ProxyingURLLoaderFactory::CreateLoaderAndStart(
    network::mojom::URLLoaderRequest loader,
    int32_t routing_id,
    int32_t request_id,
    uint32_t options,
    const network::ResourceRequest& request,
    network::mojom::URLLoaderClientPtr client,
    const net::MutableNetworkTrafficAnnotationTag& traffic_annotation) {
  target_factory_->CreateLoaderAndStart(std::move(loader), routing_id,
                                        request_id, options, request,
                                        std::move(client), traffic_annotation);
}

void ProxyingURLLoaderFactory::Clone(
    network::mojom::URLLoaderFactoryRequest loader_request) {
  proxy_bindings_.AddBinding(this, std::move(loader_request));
}

void ProxyingURLLoaderFactory::OnTargetFactoryError() {
  delete this;
}

void ProxyingURLLoaderFactory::OnProxyBindingError() {
  if (proxy_bindings_.empty())
    delete this;
}

}  // namespace atom
