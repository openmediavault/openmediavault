# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2024 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.

# Documentation/Howto:
# https://docs.k3s.io/installation/packaged-components
# https://docs.k3s.io/helm
# https://github.com/k3s-io/k3s/issues/1086#issuecomment-1342838441
# https://qdnqn.com/how-to-configure-traefik-on-k3s/
# https://community.traefik.io/t/adding-entrypoints-to-a-helm-deployed-traefik-on-k3s/14813/5
# https://kubernetes.io/docs/concepts/storage/volumes/#hostpath-volume-types
# https://kubernetes.io/docs/concepts/storage/persistent-volumes/#persistent-volumes

{% set k8s_config = salt['omv_conf.get']('conf.service.k8s') %}
{% set dns_config = salt['omv_conf.get']('conf.system.network.dns') %}
# {% set email_config = salt['omv_conf.get']('conf.system.notification.email') %}

{% set fqdn = dns_config.hostname | lower %}
{% if dns_config.domainname | length > 0 %}
{% set fqdn = [dns_config.hostname, dns_config.domainname] | join('.') | lower %}
{% endif %}

create_k3s_manifest_dir:
  file.directory:
    - name: "/var/lib/rancher/k3s/server/manifests/"
    - makedirs: True

create_k3s_traefik_manifest:
  file.managed:
    - name: "/var/lib/rancher/k3s/server/manifests/openmediavault-traefik.yaml"
    - contents: |
        ---
        apiVersion: helm.cattle.io/v1
        kind: HelmChartConfig
        metadata:
          name: traefik
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          valuesContent: |-
            ports:
              web:
                exposedPort: {{ k8s_config.webport }}
              websecure:
                exposedPort: {{ k8s_config.websecureport }}
              dashboard:
                port: {{ k8s_config.dashboardport }}
                protocol: TCP
                expose: true
                exposedPort: {{ k8s_config.dashboardport }}
                tls:
                  enabled: true
        ---
        apiVersion: traefik.containo.us/v1alpha1
        kind: Middleware
        metadata:
          name: https-redirect
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          redirectScheme:
            scheme: https
            permanent: true
            port: "{{ k8s_config.websecureport }}"
        ---
        apiVersion: traefik.containo.us/v1alpha1
        kind: TLSStore
        metadata:
          name: default
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
{%- if k8s_config.sslcertificateref | length > 0 %}
          certificates:
            - secretName: host-imported-cert
          defaultCertificate:
            secretName: host-imported-cert
{%- else %}
          certificates:
            - secretName: host-selfsigned-cert
          defaultCertificate:
            secretName: host-selfsigned-cert
{%- endif %}
    - user: root
    - group: root
    - mode: 600

create_k3s_cert_manager_manifest:
  file.managed:
    - name: "/var/lib/rancher/k3s/server/manifests/openmediavault-cert-manager.yaml"
    - contents: |
        ---
        apiVersion: v1
        kind: Namespace
        metadata:
          name: cert-manager
          labels:
            app.kubernetes.io/part-of: openmediavault
        ---
        apiVersion: helm.cattle.io/v1
        kind: HelmChart
        metadata:
          name: cert-manager
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          repo: https://charts.jetstack.io
          chart: cert-manager
          targetNamespace: cert-manager
          valuesContent: |-
            installCRDs: true
        ---
        apiVersion: cert-manager.io/v1
        kind: ClusterIssuer
        metadata:
          name: selfsigned
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          selfSigned: {}
        # ---
        # apiVersion: cert-manager.io/v1
        # kind: ClusterIssuer
        # metadata:
        #   name: letsencrypt-staging
        #  labels:
        #    app.kubernetes.io/part-of: openmediavault
        # spec:
        #   acme:
        #     server: https://acme-staging-v02.api.letsencrypt.org/directory
        #     email: {{ email_config.sender }}
        #     privateKeySecretRef:
        #       name: letsencrypt-issuer-account-key
        #     preferredChain: ""
        #     solvers:
        #       - http01:
        #         ingress:
        #           class: traefik
        ---
        apiVersion: cert-manager.io/v1
        kind: Certificate
        metadata:
          name: host-selfsigned
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          commonName: {{ fqdn }}
          secretName: host-selfsigned-cert
          issuerRef:
            name: selfsigned
            kind: ClusterIssuer
        # ---
        # apiVersion: cert-manager.io/v1
        # kind: Certificate
        # metadata:
        #   name: host-letsencrypt-staging
        #  labels:
        #    app.kubernetes.io/part-of: openmediavault
        # spec:
        #   commonName: {{ fqdn }}
        #   dnsNames:
        #     - {{ fqdn }}
        #   secretName: host-letsencrypt-staging-cert
        #   issuerRef:
        #     name: letsencrypt-staging
        #     kind: ClusterIssuer
    - user: root
    - group: root
    - mode: 600

create_k3s_k8s_dashboard_manifest:
  file.managed:
    - name: "/var/lib/rancher/k3s/server/manifests/openmediavault-k8s-dashboard.yaml"
    - contents: |
        ---
        apiVersion: v1
        kind: Namespace
        metadata:
          name: kubernetes-dashboard
          labels:
            app.kubernetes.io/part-of: openmediavault
        ---
        apiVersion: helm.cattle.io/v1
        kind: HelmChart
        metadata:
          name: kubernetes-dashboard
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          repo: https://kubernetes.github.io/dashboard/
          chart: kubernetes-dashboard
          targetNamespace: kubernetes-dashboard
          valuesContent: |-
            cert-manager:
              enabled: false
            nginx:
              enabled: false
            app:
              ingress:
                enabled: false
        ---
        apiVersion: traefik.containo.us/v1alpha1
        kind: ServersTransport
        metadata:
          name: no-verify-tls
          namespace: kubernetes-dashboard
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          insecureSkipVerify: true
        ---
        apiVersion: traefik.containo.us/v1alpha1
        kind: IngressRoute
        metadata:
          name: kubernetes-dashboard
          namespace: kubernetes-dashboard
          labels:
            app.kubernetes.io/part-of: openmediavault
        spec:
          entryPoints:
            - dashboard
          routes:
            - match: PathPrefix(`/`)
              kind: Rule
              services:
                - name: kubernetes-dashboard-kong-proxy
                  port: 443
                  serversTransport: no-verify-tls
        ---
        apiVersion: v1
        kind: ServiceAccount
        metadata:
          name: admin-user
          namespace: kubernetes-dashboard
          labels:
            app.kubernetes.io/part-of: openmediavault
        ---
        apiVersion: rbac.authorization.k8s.io/v1
        kind: ClusterRoleBinding
        metadata:
          name: admin-user
          labels:
            app.kubernetes.io/part-of: openmediavault
        roleRef:
          apiGroup: rbac.authorization.k8s.io
          kind: ClusterRole
          name: cluster-admin
        subjects:
        - kind: ServiceAccount
          name: admin-user
          namespace: kubernetes-dashboard
        ---
        apiVersion: v1
        kind: Secret
        metadata:
          name: admin-user
          namespace: kubernetes-dashboard
          annotations:
            kubernetes.io/service-account.name: "admin-user"
          labels:
            app.kubernetes.io/part-of: openmediavault
        type: kubernetes.io/service-account-token
    - user: root
    - group: root
    - mode: 600

create_k3s_misc_manifest:
  file.managed:
    - name: "/var/lib/rancher/k3s/server/manifests/openmediavault-misc.yaml"
    - contents: |
{%- if k8s_config.sslcertificateref | length > 0 %}
{%- set ssl_cert_dir = salt['pillar.get']('default:OMV_SSL_CERTIFICATE_DIR', '/etc/ssl') %}
{%- set ssl_cert_prefix = salt['pillar.get']('default:OMV_SSL_CERTIFICATE_PREFIX', 'openmediavault') %}
        ---
        apiVersion: v1
        kind: Secret
        metadata:
          name: host-imported-cert
          namespace: kube-system
          labels:
            app.kubernetes.io/part-of: openmediavault
        data:
          tls.crt: {{ ssl_cert_dir | path_join('certs', ssl_cert_prefix ~ k8s_config.sslcertificateref ~ '.crt') | file_base64_encode }}
          tls.key: {{ ssl_cert_dir | path_join('private', ssl_cert_prefix ~ k8s_config.sslcertificateref ~ '.key') | file_base64_encode }}
        type: kubernetes.io/tls
{%- endif %}
    - user: root
    - group: root
    - mode: 600

create_k3s_local_storage_manifest:
  file.managed:
    - name: "/var/lib/rancher/k3s/server/manifests/openmediavault-local-storage.yaml"
    - contents: |
        ---
        apiVersion: storage.k8s.io/v1
        kind: StorageClass
        metadata:
          name: shared-folder
          labels:
            app.kubernetes.io/part-of: openmediavault
        provisioner: rancher.io/local-path
        reclaimPolicy: Retain
        volumeBindingMode: WaitForFirstConsumer
    - user: root
    - group: root
    - mode: 600

{% if k8s_config.enable | to_bool %}

install_k3s:
  cmd.run:
    - name: set -o pipefail; wget -O - https://get.k3s.io | INSTALL_K3S_SKIP_ENABLE=true sh -
    - shell: /usr/bin/bash
    - onlyif: "! which k3s || test -e /var/lib/openmediavault/upgrade_k3s"
    - failhard: True

remove_k3s_upgrade_flag:
  file.absent:
    - name: "/var/lib/openmediavault/upgrade_k3s"

# remove_k3s_helm_upgrade_flag:
#   file.absent:
#     - name: "/var/lib/openmediavault/upgrade_helm"

start_k3s_service:
  service.running:
    - name: k3s
    - enable: True

{% else %}

stop_k3s_service:
  service.dead:
    - name: k3s
    - enable: False

{% endif %}
