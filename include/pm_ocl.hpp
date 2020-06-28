/*!
  \file
  \brief Параллельная реализация фильтра Перона-Малика
  \author Ilya Shoshin (Galarius)
  \copyright (c) 2016, Research Institute of Instrument Engineering
*/

#ifndef __pm_ocl_hpp__
#define __pm_ocl_hpp__

extern "C" {
#include "pm.h" // img_data, proc_data
}

#include <string>

typedef struct {
    /*!\{*/
    int platformId; ///< индекс платформы
    int deviceId;   ///< индекс устройства
    bool profile;   ///< включить профилирование?
    std::string filename; ///< имя файла (kernel или биткод)
    bool bitcode;   ///< filename указывает на биткод?
    bool verbose;   ///< подробный вывод
    /*!\}*/
} cl_data;  /*! параметры OpenCL */

/*!
 * Параллельное выполнение фильтра Перона-Малика
 *
 * \param idata - данные изображения
 * \param pdata - параметры фильтра
 * \param cdata - параметры opencl
 * \throws cl::Error
 * \throws std::runtime_error
 * \throws std::invalid_argument
 * \see img_data
 * \see proc_data
 * \see cl_data
 *
 * ПРИМЕР:
 * \code{cpp}

  // Данные изображения и параметры обработки
  img_data idata = {
      packed_data, // упакованные rgb
      packed_size, // размер packed_data в байтах
      input_img.width, input_img.height  // ширина, высота в px
  };
  // Параметры фильтра
  proc_data pdata = {
      iterations,           // кол-во итераций
      conduction_function,  // функция для вычисления проводимости [0, 1]
      NULL,                 // не используется
      thresh,               // пороговое значение
      lambda                // коэффициент Лапласиана
  };
  // Настройка OpenCL
  cl_data cdata = {
      platformId,           // номер платформы
      deviceId,             // номер устройства
      false,                // профилировать?
      kernel_file,          // файл с кодом ядра / биткодом
      false,                // биткод?
      true                  // выводить детализированную информацию?
  };

  try
  {
    pm_parallel(&idata, &pdata, &cdata);
  } catch (cl::Error err) {
      std::cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << std::endl;
  } catch(std::invalid_argument e) {
      std::cerr << e.what();
  } catch(std::runtime_error e) {
      std::cerr << e.what();
  }
 * \endcode
 */
void pm_parallel(img_data *idata, proc_data *pdata, cl_data *cdata);

#endif  /* __pm_ocl_hpp__ */