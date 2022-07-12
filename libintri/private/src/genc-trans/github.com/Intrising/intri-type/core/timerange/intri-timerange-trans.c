// Code generated by protoc-gen-yang(*.c) DO NOT EDIT.

/*****************************************************************************************************
 * Copyright (C) 2017-2022 by Intrising
 *  - ian0113@intrising.com.tw
 * 
 * Generated by protoc-gen-yang@gen-yang
 * 
 *****************************************************************************************************/

#include <libxml/xmlstring.h>
#include "agt.h"
#include "agt_cb.h"
#include "agt_rpc.h"
#include "agt_timer.h"
#include "agt_util.h"
#include "dlq.h"
#include "ncx.h"
#include "ncx_feature.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "procdefs.h"
#include "rpc.h"
#include "ses.h"
#include "status.h"
#include "val.h"
#include "val_util.h"
#include "xml_util.h"

#include "intri-timerange-trans.h"
#include "../../../../../../../../.libintrishare/libintrishare.h"

#include "../../../../../github.com/Intrising/intri-type/common/intri-common-trans.h"
#include "../../../../../github.com/golang/protobuf/ptypes/empty/intri-empty-trans.h"
#include "../../../../../google.golang.org/genproto/googleapis/type/date/intri-date-trans.h"
#include "../../../../../google.golang.org/genproto/googleapis/type/dayofweek/intri-dayofweek-trans.h"
#include "../../../../../google.golang.org/genproto/googleapis/type/timeofday/intri-timeofday-trans.h"

status_t build_to_xml_timerange_Config(
    val_value_t *parentval,
    struct timerangepb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "List",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  for (int i = 0; i < entry->List_Len; i++) {
    val_value_t *listval = NULL;
    listval = agt_make_object(
        childval->obj,
        "List_Entry",
        &res);
    if (listval != NULL) {
      val_add_child(listval, childval);
    } else if (res != NO_ERR) {
      return SET_ERROR(res);
    }
    /* message */
    res = build_to_xml_timerange_ConfigEntry(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_xml_timerange_ConfigEntry(
    val_value_t *parentval,
    struct timerangepb_ConfigEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Name",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Name;
  childval = agt_make_object(
      parentval->obj,
      "Enabled",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->Enabled;
  childval = agt_make_object(
      parentval->obj,
      "Command",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->Command) {
    case timerangepb_CommandTypeOptions_COMMAND_TYPE_PERIODIC:
      enum_str = "COMMAND_TYPE_PERIODIC";
      break;
    case timerangepb_CommandTypeOptions_COMMAND_TYPE_ABSOLUTE:
      enum_str = "COMMAND_TYPE_ABSOLUTE";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  childval = agt_make_object(
      parentval->obj,
      "PeriodicDay",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_timerange_DayTime(
      childval,
      entry->PeriodicDay);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "StartTime",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_timeofday_TimeOfDay(
      childval,
      entry->StartTime);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "EndTime",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_timeofday_TimeOfDay(
      childval,
      entry->EndTime);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "StartDate",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_date_Date(
      childval,
      entry->StartDate);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  childval = agt_make_object(
      parentval->obj,
      "EndDate",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* message */
  res = build_to_xml_date_Date(
      childval,
      entry->EndDate);
  if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  return res;
}
status_t build_to_xml_timerange_DayTime(
    val_value_t *parentval,
    struct timerangepb_DayTime *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "PeriodicType",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* enum */
  switch (entry->PeriodicType) {
    case timerangepb_PeriodicDayTypeOptions_PERIODIC_DAY_TYPE_DAYOFWEEK:
      enum_str = "PERIODIC_DAY_TYPE_DAYOFWEEK";
      break;
    case timerangepb_PeriodicDayTypeOptions_PERIODIC_DAY_TYPE_DAILY:
      enum_str = "PERIODIC_DAY_TYPE_DAILY";
      break;
    case timerangepb_PeriodicDayTypeOptions_PERIODIC_DAY_TYPE_WEEKDAYS:
      enum_str = "PERIODIC_DAY_TYPE_WEEKDAYS";
      break;
    case timerangepb_PeriodicDayTypeOptions_PERIODIC_DAY_TYPE_WEEKEND:
      enum_str = "PERIODIC_DAY_TYPE_WEEKEND";
      break;
  }
  VAL_ENUM_NAME(childval) = enum_str;
  childval = agt_make_object(
      parentval->obj,
      "DayOfWeekLists",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  for (int i = 0; i < entry->DayOfWeekLists_Len; i++) {
    val_value_t *listval = NULL;
    listval = agt_make_object(
        childval->obj,
        "DayOfWeekLists_Entry",
        &res);
    if (listval != NULL) {
      val_add_child(listval, childval);
    } else if (res != NO_ERR) {
      return SET_ERROR(res);
    }
    /* enum */
    switch (entry->DayOfWeekLists[i]) {
      case dayofweekpb_DayOfWeek_DAY_OF_WEEK_UNSPECIFIED:
        enum_str = "DAY_OF_WEEK_UNSPECIFIED";
        break;
      case dayofweekpb_DayOfWeek_MONDAY:
        enum_str = "MONDAY";
        break;
      case dayofweekpb_DayOfWeek_TUESDAY:
        enum_str = "TUESDAY";
        break;
      case dayofweekpb_DayOfWeek_WEDNESDAY:
        enum_str = "WEDNESDAY";
        break;
      case dayofweekpb_DayOfWeek_THURSDAY:
        enum_str = "THURSDAY";
        break;
      case dayofweekpb_DayOfWeek_FRIDAY:
        enum_str = "FRIDAY";
        break;
      case dayofweekpb_DayOfWeek_SATURDAY:
        enum_str = "SATURDAY";
        break;
      case dayofweekpb_DayOfWeek_SUNDAY:
        enum_str = "SUNDAY";
        break;
    }
    VAL_ENUM_NAME(listval) = enum_str;
  }
  return res;
}
status_t build_to_xml_timerange_EntryStatus(
    val_value_t *parentval,
    struct timerangepb_EntryStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "Name",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* string */
  VAL_STRING(childval) = entry->Name;
  childval = agt_make_object(
      parentval->obj,
      "IsActive",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  /* bool */
  VAL_BOOL(childval) = entry->IsActive;
  return res;
}
status_t build_to_xml_timerange_Status(
    val_value_t *parentval,
    struct timerangepb_Status *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  const xmlChar *enum_str = EMPTY_STRING;
  if (entry == NULL) {
    return res;
  }
  childval = agt_make_object(
      parentval->obj,
      "List",
      &res);
  if (childval != NULL) {
    val_add_child(childval, parentval);
  } else if (res != NO_ERR) {
    return SET_ERROR(res);
  }
  for (int i = 0; i < entry->List_Len; i++) {
    val_value_t *listval = NULL;
    listval = agt_make_object(
        childval->obj,
        "List_Entry",
        &res);
    if (listval != NULL) {
      val_add_child(listval, childval);
    } else if (res != NO_ERR) {
      return SET_ERROR(res);
    }
    /* message */
    res = build_to_xml_timerange_EntryStatus(
        listval,
        entry->List[i]);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}

status_t build_to_priv_timerange_Config(
    val_value_t *parentval,
    struct timerangepb_Config *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "List");
  entry->List_Len = 0;
  entry->List = malloc(sizeof(*entry->List));
  if (childval != NULL && childval->res == NO_ERR) {
    entry->List_Len = dlq_count(&childval->v.childQ);
    entry->List = malloc((entry->List_Len + 1) * sizeof(*entry->List));
    unsigned int cnt = 0;
    val_value_t *listval = NULL;
    for (listval = (val_value_t *)dlq_firstEntry(&childval->v.childQ);
         listval != NULL;
         listval = (val_value_t *)dlq_nextEntry(listval)) {
      /* message */
      entry->List[cnt] = malloc(sizeof(*(entry->List[cnt])));
      res = build_to_priv_timerange_ConfigEntry(
          listval,
          entry->List[cnt]);
      if (res != NO_ERR) {
        return SET_ERROR(res);
      }
      cnt++;
    }
  }
  return res;
}
status_t build_to_priv_timerange_ConfigEntry(
    val_value_t *parentval,
    struct timerangepb_ConfigEntry *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Name");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->Name = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Enabled");
  if (childval != NULL && childval->res == NO_ERR) {
    /* bool */
    entry->Enabled = VAL_BOOL(childval);
  }
  childval = val_first_child_name(
      parentval,
      "Command");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->Command = VAL_ENUM(childval);
  }
  childval = val_first_child_name(
      parentval,
      "PeriodicDay");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->PeriodicDay = malloc(sizeof(*(entry->PeriodicDay)));
    res = build_to_priv_timerange_DayTime(
        childval,
        entry->PeriodicDay);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "StartTime");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->StartTime = malloc(sizeof(*(entry->StartTime)));
    res = build_to_priv_timeofday_TimeOfDay(
        childval,
        entry->StartTime);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "EndTime");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->EndTime = malloc(sizeof(*(entry->EndTime)));
    res = build_to_priv_timeofday_TimeOfDay(
        childval,
        entry->EndTime);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "StartDate");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->StartDate = malloc(sizeof(*(entry->StartDate)));
    res = build_to_priv_date_Date(
        childval,
        entry->StartDate);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  childval = val_first_child_name(
      parentval,
      "EndDate");
  if (childval != NULL && childval->res == NO_ERR) {
    /* message */
    entry->EndDate = malloc(sizeof(*(entry->EndDate)));
    res = build_to_priv_date_Date(
        childval,
        entry->EndDate);
    if (res != NO_ERR) {
      return SET_ERROR(res);
    }
  }
  return res;
}
status_t build_to_priv_timerange_DayTime(
    val_value_t *parentval,
    struct timerangepb_DayTime *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "PeriodicType");
  if (childval != NULL && childval->res == NO_ERR) {
    /* enum */
    entry->PeriodicType = VAL_ENUM(childval);
  }
  childval = val_first_child_name(
      parentval,
      "DayOfWeekLists");
  entry->DayOfWeekLists_Len = 0;
  entry->DayOfWeekLists = malloc(sizeof(*entry->DayOfWeekLists));
  if (childval != NULL && childval->res == NO_ERR) {
    entry->DayOfWeekLists_Len = dlq_count(&childval->v.childQ);
    entry->DayOfWeekLists = malloc((entry->DayOfWeekLists_Len + 1) * sizeof(*entry->DayOfWeekLists));
    unsigned int cnt = 0;
    val_value_t *listval = NULL;
    for (listval = (val_value_t *)dlq_firstEntry(&childval->v.childQ);
         listval != NULL;
         listval = (val_value_t *)dlq_nextEntry(listval)) {
      /* enum */
      entry->DayOfWeekLists[cnt] = VAL_ENUM(listval);
      cnt++;
    }
  }
  return res;
}
status_t build_to_priv_timerange_EntryStatus(
    val_value_t *parentval,
    struct timerangepb_EntryStatus *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "Name");
  if (childval != NULL && childval->res == NO_ERR) {
    /* string */
    entry->Name = VAL_STRING(childval);
  }
  childval = val_first_child_name(
      parentval,
      "IsActive");
  if (childval != NULL && childval->res == NO_ERR) {
    /* bool */
    entry->IsActive = VAL_BOOL(childval);
  }
  return res;
}
status_t build_to_priv_timerange_Status(
    val_value_t *parentval,
    struct timerangepb_Status *entry) {
  status_t res = NO_ERR;
  val_value_t *childval = NULL;
  childval = val_first_child_name(
      parentval,
      "List");
  entry->List_Len = 0;
  entry->List = malloc(sizeof(*entry->List));
  if (childval != NULL && childval->res == NO_ERR) {
    entry->List_Len = dlq_count(&childval->v.childQ);
    entry->List = malloc((entry->List_Len + 1) * sizeof(*entry->List));
    unsigned int cnt = 0;
    val_value_t *listval = NULL;
    for (listval = (val_value_t *)dlq_firstEntry(&childval->v.childQ);
         listval != NULL;
         listval = (val_value_t *)dlq_nextEntry(listval)) {
      /* message */
      entry->List[cnt] = malloc(sizeof(*(entry->List[cnt])));
      res = build_to_priv_timerange_EntryStatus(
          listval,
          entry->List[cnt]);
      if (res != NO_ERR) {
        return SET_ERROR(res);
      }
      cnt++;
    }
  }
  return res;
}
