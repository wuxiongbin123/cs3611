#!/bin/bash

for ((i=1; i<10; i++))
do
  # 启动prog程序，并将其放到后台运行
  ./prog &

  # 可选：再次等待一段时间，确保prog程序已经启动
  sleep 1
done