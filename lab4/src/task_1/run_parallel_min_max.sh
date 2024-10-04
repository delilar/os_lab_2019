#!/bin/bash

# Проверяем, что makefile собрал программу parallel_min_max
if [ ! -f "./parallel_min_max" ]; then
  echo "parallel_min_max not found, compiling with make..."
  make parallel_min_max
  if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
  fi
fi

# Устанавливаем значения по умолчанию
SEED=1
ARRAY_SIZE=1000
PNUM=4
TIMEOUT=10  # Таймаут по умолчанию 10 секунд
USE_FILES=false  # По умолчанию используем пайпы, если не указано by_files

# Обрабатываем аргументы
while [[ $# -gt 0 ]]; do
  case $1 in
    --seed)
      SEED="$2"
      shift # пропускаем значение
      ;;
    --array_size)
      ARRAY_SIZE="$2"
      shift
      ;;
    --pnum)
      PNUM="$2"
      shift
      ;;
    --timeout)
      TIMEOUT="$2"
      shift
      ;;
    --by_files)
      USE_FILES=true
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
  shift # Переходим к следующему аргументу
done

# Собираем команду для запуска
CMD="./parallel_min_max --seed $SEED --array_size $ARRAY_SIZE --pnum $PNUM --timeout $TIMEOUT"
if $USE_FILES; then
  CMD="$CMD --by_files"
fi

echo "Running command: $CMD"
$CMD
