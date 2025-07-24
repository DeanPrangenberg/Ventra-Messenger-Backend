#!/bin/sh

# Change to script directory
cd "$(dirname "$0")" || exit 1

K8S_DIR="k8s"

echo "Deleting existing Minikube cluster..."
minikube delete >/dev/null 2>&1 || true

echo "Starting Minikube..."
minikube start --driver=docker

echo "Cleaning up previous Kubernetes manifests..."
rm -rf "$K8S_DIR"
mkdir "$K8S_DIR"

echo "Converting docker-compose.yml to Kubernetes manifests..."
kompose convert --out "$K8S_DIR"

echo "Applying manifests to the Minikube cluster..."
kubectl apply -f "$K8S_DIR"

echo "Showing cluster status:"
kubectl get all

