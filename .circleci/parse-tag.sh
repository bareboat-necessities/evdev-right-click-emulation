#!/bin/bash

  BASH_ENV=/tmp/.bash_env-5f8f6d969b9288123e04df41-0-build
  CI=true
  CIRCLECI=true
  CIRCLE_BRANCH=hold-right-click
  CIRCLE_BUILD_NUM=14
  CIRCLE_BUILD_URL=https://circleci.com/gh/bareboat-necessities/evdev-right-click-emulation/14
  CIRCLE_COMPARE_URL=
  CIRCLE_JOB=build-arm64
  CIRCLE_NODE_INDEX=0
  CIRCLE_NODE_TOTAL=1
  CIRCLE_PREVIOUS_BUILD_NUM=13
  CIRCLE_PROJECT_REPONAME=evdev-right-click-emulation
  CIRCLE_PROJECT_USERNAME=bareboat-necessities
  CIRCLE_REPOSITORY_URL=git@github.com:bareboat-necessities/evdev-right-click-emulation.git
  CIRCLE_SHA1=27a549c76c4baddc345414cb8fbca2ddcf1ccbcf
  CIRCLE_SHELL_ENV=/tmp/.bash_env-5f8f6d969b9288123e04df41-0-build
  CIRCLE_STAGE=build-arm64
  CIRCLE_USERNAME=mgrouch
  CIRCLE_WORKFLOW_ID=d3c40c47-b1b5-4a3e-8a99-1ad44d5e88ce
  CIRCLE_WORKFLOW_JOB_ID=a0353f92-e4f9-4158-b7c7-fd199d38ec4e
  CIRCLE_WORKFLOW_UPSTREAM_JOB_IDS=
  CIRCLE_WORKFLOW_WORKSPACE_ID=d3c40c47-b1b5-4a3e-8a99-1ad44d5e88ce
  CIRCLE_WORKING_DIRECTORY=~/project


SEPARATOR=$(expr index "$CIRCLE_TAG" "/")
NODE_NAME=${CIRCLE_TAG:0:$SEPARATOR-1}
if expr index "$CIRCLE_TAG" "-" > /dev/null; then
SEPARATOR_REVISION=$(expr index "$CIRCLE_TAG" "-")
REVISION="${CIRCLE_TAG:$SEPARATOR_REVISION}"
NODE_VERSION="${CIRCLE_TAG:$SEPARATOR:$SEPARATOR_REVISION-$SEPARATOR-1}"
else
REVISION=""
NODE_VERSION="${CIRCLE_TAG:$SEPARATOR}"
fi
LATEST_TAG="${CIRCLE_TAG:$SEPARATOR}"
DOCKERHUB_REPO="btcpayserver/$NODE_NAME"
DOCKERHUB_REPO="${DOCKERHUB_REPO,,}"
DOCKERHUB_DESTINATION="$DOCKERHUB_REPO:$LATEST_TAG"
DOCKERHUB_DOCKEFILE_ARM64="$NODE_NAME/$NODE_VERSION/linuxarm64v8.Dockerfile"
DOCKERHUB_DOCKEFILE_ARM32="$NODE_NAME/$NODE_VERSION/linuxarm32v7.Dockerfile"
DOCKERHUB_DOCKEFILE_AMD64="$NODE_NAME/$NODE_VERSION/linuxamd64.Dockerfile"

echo "LATEST_TAG=$LATEST_TAG"
echo "NODE_VERSION=$NODE_VERSION"
echo "REVISION=$REVISION"
echo "DOCKERHUB_REPO=$DOCKERHUB_REPO"
echo "DOCKERHUB_DESTINATION=$DOCKERHUB_DESTINATION"
echo "DOCKERHUB_DOCKEFILE_AMD64=$DOCKERHUB_DOCKEFILE_AMD64"
echo "DOCKERHUB_DOCKEFILE_ARM32=$DOCKERHUB_DOCKEFILE_ARM32"
echo "DOCKERHUB_DOCKEFILE_ARM64=$DOCKERHUB_DOCKEFILE_ARM64"