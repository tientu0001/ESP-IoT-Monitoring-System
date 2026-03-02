
#ifndef FUN_OPER_H
#define FUN_OPER_H
//************************************************************************************************
#include "config.h"
// #include <stdint.h>
// #include <WString.h>

//************************************************************************************************
class FunOperation
{
public:
  // function
  void init();
  void loop();

  void increase_output();
  void increase_defect();
  void reset_output();
  void reset_defect();
  void addQueue_product_data(uint8_t ucType, uint32_t ulData);
  void delQueue_product_data(uint8_t *ucOutBuff);
  void dev_IncQfontIdx();
  void dev_IncQrearIdx();
  void update_product_current_process_id();
  void update_config_current_process_id();

  // in/out iot mode
  bool in_out_iot_mode_enable;
  bool in_out_input_mode;

  uint32_t target, output, defect;
  uint16_t scale;

  String factory, line;
  uint16_t position;
  // uint8_t process;

  String machine_id;
  String worker_id, worker_name, worker_skill;
  String style, process, process_id;

  uint8_t len_plan;
  uint8_t current_idx_plan;
  uint32_t handling_time_idx_plan;

  uint16_t list_addr_output[LEN_JSON_OBJECT_PLAN];

  String plan_wrk_id[LEN_JSON_OBJECT_PLAN];
  String plan_stlye[LEN_JSON_OBJECT_PLAN];
  String plan_process[LEN_JSON_OBJECT_PLAN];
  String plan_process_id[LEN_JSON_OBJECT_PLAN];
  // uint16_t plan_position[LEN_JSON_OBJECT_PLAN];

  uint32_t plan_handling_time[LEN_JSON_OBJECT_PLAN];
  uint16_t plan_scale[LEN_JSON_OBJECT_PLAN];

  bool bRecRealPlan;

  bool bGetDatetime;
  bool bGetPlan;
  bool bGetConfig;

  uint32_t power_ontime;       // power ontime
  uint32_t motor_runtime;      // motor runtime
  uint32_t handling_time;      // handling time
  uint32_t cycle_time;         // cycle time
  uint32_t current_cycle_time; // cycle time between 2 output happned

  uint32_t sumof_stitch; // number of stitch
  uint32_t sumof_trim;   // number of stitch

  uint32_t power_time_line;
  uint32_t power_time_reset;

  // check & calculate motor runtime
  bool bMotorRun;
  uint32_t ulCurMotRunMs;
  uint32_t ulTimeSen0HappnedFirst;
  uint32_t ulTimeSen0HappnedFinal;

  uint32_t u32_stitch_happend;

  uint8_t ucNonSewingMc;
  uint32_t ulGenScanOld;

  uint8_t daily_product_reset;

  bool bSendMcTime;

  bool bAndonMode;
  bool bSendAndon;

  bool dir_andon_mc_mode;

  bool check_data_machine;

private:
  // handling time
  bool bCalHandlingTime;
  uint32_t ulCurHanTime;
  uint32_t ulHanTimeTmp;
};

//************************************************************************************************
extern FunOperation oper;

//************************************************************************************************
#endif // FUN_OPER_H