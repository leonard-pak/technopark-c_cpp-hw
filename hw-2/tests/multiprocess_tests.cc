/**
 * lcov -t "tests/tests_kmeans" -o coverage.info -c -d kmeans_libs/
 * genhtml -o report coverage.info
 * cd report && python3 -m http.server 8000
 */
#include "gtest/gtest.h"

extern "C" {
#include "kmeans_multiprocess.h"
}

TEST(MULTIPROCESS_TESTS, CreatPoints_TEST) {
  KMeans* kmeans = NULL;
  int status = system("data/data 2 /tmp/data_false.bin");
  ASSERT_EQ(status, 0);
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, CreatPoints(NULL, "/tmp/data_false.bin"));
  EXPECT_EQ(FAILURE, CreatPoints(&kmeans, NULL));
  // Проверка на неправильный файл
  EXPECT_EQ(FAILURE, CreatPoints(&kmeans, "no_file.bin"));
  EXPECT_EQ(FAILURE, CreatPoints(&kmeans, ""));
  // Проверка на неправильные данные
  EXPECT_EQ(FAILURE, CreatPoints(&kmeans, "/tmp/data_false.bin"));
}

TEST(MULTIPROCESS_TESTS, StartAlgorithm_TEST) {
  KMeans* kmeans = (KMeans*)malloc(1 * sizeof(KMeans));
  kmeans->points_cnt = 3;
  kmeans->clusters_cnt = 2;
  // Проверка на передачу неправилных данных
  EXPECT_EQ(FAILURE, StartAlgorithm(NULL));
  kmeans->points_cnt = 0;
  EXPECT_EQ(FAILURE, StartAlgorithm(kmeans));
  kmeans->points_cnt = 1;
  kmeans->clusters_cnt = 0;
  EXPECT_EQ(FAILURE, StartAlgorithm(kmeans));
  kmeans->clusters_cnt = 2;
  EXPECT_EQ(FAILURE, StartAlgorithm(kmeans));
  // Освобождение памяти
  free(kmeans);
}

TEST(MULTIPROCESS_TESTS, DeletePoints_TEST) {
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, DeletePoints(NULL));
}

TEST(MULTIPROCESS_TESTS, SendMessage_TEST) {
  char const* text = "";
  // Проверка на передачу неверных данных
  EXPECT_EQ(FAILURE, SendMessage(0, 0, NULL));
  EXPECT_EQ(FAILURE, SendMessage(0, 0, text));
}

TEST(MULTIPROCESS_TESTS, ReadMessage_TEST) {
  char* text = (char*)calloc(1, sizeof(char));
  // Проверка на передачу неверных данных
  EXPECT_EQ(FAILURE, ReadMessage(0, NULL, 0));
  EXPECT_EQ(FAILURE, ReadMessage(0, text, 0));
  // Освобождение памяти
  free(text);
}

TEST(MULTIPROCESS_TESTS, StartChildWork_TEST) {
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, StartChildWork(0, NULL));
}

TEST(MULTIPROCESS_TESTS, InitProcesses_TEST) {
  KMeans* kmeans = (KMeans*)malloc(1 * sizeof(KMeans));
  int msgid = 0;
  int pids[] = {0};
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, InitProcesses(NULL, &msgid, pids, 0));
  EXPECT_EQ(FAILURE, InitProcesses(kmeans, NULL, pids, 0));
  EXPECT_EQ(FAILURE, InitProcesses(kmeans, &msgid, NULL, 0));
  // Освобождение памяти
  free(kmeans);
}

TEST(MULTIPROCESS_TESTS, PhaseSortClusters_TEST) {
  KMeans* kmeans = (KMeans*)malloc(1 * sizeof(KMeans));
  size_t changed = 0;
  int pids[] = {0};
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, PhaseSortClusters(NULL, 0, 1, pids, &changed));
  EXPECT_EQ(FAILURE, PhaseSortClusters(kmeans, 0, 1, NULL, &changed));
  EXPECT_EQ(FAILURE, PhaseSortClusters(kmeans, 0, 1, pids, NULL));
  EXPECT_EQ(FAILURE, PhaseSortClusters(kmeans, 0, 0, pids, &changed));
  // Освобождение памяти
  free(kmeans);
}

TEST(MULTIPROCESS_TESTS, PhaseFindCenter_TEST) {
  KMeans* kmeans = (KMeans*)malloc(1 * sizeof(KMeans));
  int pids[] = {0};
  // Проверка на передачу NULL
  EXPECT_EQ(FAILURE, PhaseFindCenter(NULL, 0, 1, pids));
  EXPECT_EQ(FAILURE, PhaseFindCenter(kmeans, 0, 1, NULL));
  EXPECT_EQ(FAILURE, PhaseFindCenter(kmeans, 0, 0, pids));
  // Освобождение памяти
  free(kmeans);
}

TEST(MULTIPROCESS_TESTS, MAIN_TEST) {
  int status = system("data/data 1 /tmp/data.bin");
  ASSERT_EQ(status, 0);
  status = system("../hw-2 /tmp/data.bin /tmp/out_data.bin");
  ASSERT_EQ(status, 0);

  FILE* fptr = NULL;
  fptr = fopen("/tmp/out_data.bin", "rb");
  if (fptr == NULL) {
    FAIL();
  }
  size_t clusters_cnt = 0;
  if (fread(&clusters_cnt, sizeof(size_t), 1, fptr) != 1) {
    fclose(fptr);
    FAIL();
  }

  int delta_x = 0;
  int delta_y = 0;
  int delta_z = 0;
  for (size_t i = 0; i < clusters_cnt; ++i) {
    size_t points_cnt = 0;
    if (fread(&points_cnt, sizeof(size_t), 1, fptr) != 1) {
      fclose(fptr);
      FAIL();
    }
    Point cluster;
    if (fread(&cluster, sizeof(Point), 1, fptr) != 1) {
      fclose(fptr);
      FAIL();
    }
    for (size_t j = 0; j < points_cnt; ++j) {
      Point point;
      if (fread(&point, sizeof(Point), 1, fptr) != 1) {
        fclose(fptr);
        FAIL();
      }
      // Тестовые данные сгенерированны так, что разброс не больше 1000, сдвиг
      // 2000. Поэтому проверка на дальность от центра на 1000 (с запасом)
      delta_x = abs(cluster.x - point.x);
      delta_y = abs(cluster.y - point.y);
      delta_z = abs(cluster.z - point.z);
      EXPECT_LT(delta_x, 1000);
      EXPECT_LT(delta_y, 1000);
      EXPECT_LT(delta_z, 1000);
    }
  }

  if (fclose(fptr)) {
    FAIL();
  }
  SUCCEED();
}

TEST(MULTIPROCESS_TESTS, STRESS_TEST) {
  int status = system("data/data 0 /tmp/data.bin");
  ASSERT_EQ(status, 0);
  status = system("../hw-2 /tmp/data.bin /tmp/out_data.bin");
  ASSERT_EQ(status, 0);
}