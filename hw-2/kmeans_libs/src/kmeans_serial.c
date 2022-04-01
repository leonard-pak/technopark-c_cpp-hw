/**
 * @file kmeans_serial.c
 * @author your name (you@domain.com)
 * @brief Запуск алгоритма k-средних в последовательном режиме
 * @version 0.1
 * @date 2022-03-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "kmeans.h"

const float threshold = 0.01;

/**
 * @brief Создает специальную структуру из TODO:
 *
 * @param kmeans - должен быть пустым и NULL
 * @return int
 */
int CreatPoints(KMeans** kmeans, char const* file_name) {
  if ((kmeans == NULL) || (*kmeans != NULL) || (file_name == NULL)) {
    return FAILURE;
  }
  // TODO: Переписать под прием из файла
  FILE* fptr = NULL;
  fptr = fopen(file_name, "rb");
  if (fptr == NULL) {
    return FAILURE;
  }
  KMeans* tmp_kmeans = (KMeans*)malloc(1 * sizeof(KMeans));
  fread(&tmp_kmeans->points_cnt, sizeof(size_t), 1, fptr);
  fread(&tmp_kmeans->clusters_cnt, sizeof(size_t), 1, fptr);

  // Кластеров не должно быть больше, чем самих точек
  if (tmp_kmeans->clusters_cnt > tmp_kmeans->points_cnt) {
    free(tmp_kmeans);
    fclose(fptr);
    return FAILURE;
  }
  tmp_kmeans->points =
      (PointInCluster*)calloc(tmp_kmeans->points_cnt, sizeof(PointInCluster));
  tmp_kmeans->clusters =
      (Point*)malloc(tmp_kmeans->clusters_cnt * sizeof(Point));
  for (size_t i = 0; i < tmp_kmeans->points_cnt; ++i) {
    fread(&tmp_kmeans->points[i].point, sizeof(Point), 1, fptr);
  }

  *kmeans = tmp_kmeans;
  fclose(fptr);
  return SUCCESS;
}

/**
 * @brief Запуск алгоритма
 *
 * @param kmeans -необходимо заранее создать через вызов CreatPoints()
 * @return int
 */
int StartAlgorithm(KMeans* kmeans) {
  if (kmeans == NULL) {
    return FAILURE;
  }

  // За первые центры кластеров берутся первые точки из данных
  for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
    kmeans->clusters[i] = kmeans->points[i].point;
  }

  size_t changed = 0;
  do {
    if (ClusterSort(kmeans, 0, kmeans->points_cnt, &changed)) {
      return FAILURE;
    }
    for (size_t i = 0; i < kmeans->clusters_cnt; ++i) {
      if (FindClusterCenter(kmeans, i)) {
        return FAILURE;
      }
    }
  } while (((float)changed / (float)kmeans->points_cnt) > threshold);

  return SUCCESS;
}

/**
 * @brief Аккуратное удаление выделенной памяти
 *
 * @param kmeans
 * @return int
 */
int DeletePoints(KMeans** kmeans) {
  if ((kmeans == NULL) || (*kmeans == NULL)) {
    return FAILURE;
  }

  KMeans* tmp_kmeans = *kmeans;
  if (tmp_kmeans->points != NULL) {
    free(tmp_kmeans->points);
    tmp_kmeans->points = NULL;
  }
  if (tmp_kmeans->clusters != NULL) {
    free(tmp_kmeans->clusters);
    tmp_kmeans->clusters = NULL;
  }
  free(tmp_kmeans);
  tmp_kmeans = NULL;

  *kmeans = tmp_kmeans;
  return SUCCESS;
}
