# Cable Inventory JSON — Template and Field Guide

This document explains how to write the cable inventory JSON. The JSON describes
each cable in the system (for example CDFP and QSFP cables), how the cable and
each of its two ends are published on D-Bus, how presence is detected, where the
VPD EEPROM lives, and which inventory item each end of the cable is associated
with.

Follow the template and the field reference below to write a correct JSON for
any system.

---

## Table of contents

1. [Concepts](#1-concepts)
2. [File structure at a glance](#2-file-structure-at-a-glance)
3. [Template](#3-template)
4. [Field reference](#4-field-reference)
5. [Rules and conventions](#5-rules-and-conventions)
6. [Minimal example](#6-minimal-example)
7. [Validation checklist](#7-validation-checklist)

---

## 1. Concepts

**Cable type.** A family of cables, such as `CDFP` or `QSFP`. Each cable type is
a top-level key in the JSON, and its value is a list of cables of that type.

**Cable instance.** One physical cable, identified by an `id` that is unique
within its cable type. Each cable is published as one inventory object on D-Bus.

**Uplink and downlink.** Every cable has two ends. The **uplink** is one end and
the **downlink** is the other. Each end is published as its own inventory
object, under the cable's object path. Both ends are mandatory.

**Presence.** An optional GPIO that tells whether the cable (or one specific end
of it) is plugged in.

**EEPROM.** An optional path to the EEPROM holding VPD. Where you put the
`eeprom` field tells the code who owns that VPD:

| `eeprom` defined at… | VPD belongs to…               |
| -------------------- | ----------------------------- |
| Cable level          | The cable as a whole          |
| `uplink` level       | The uplink end of the cable   |
| `downlink` level     | The downlink end of the cable |

**Association.** An optional link between one end of the cable and another
inventory item (for example a connector or port or chassis), so that tools can
see what each end of the cable is plugged into.

---

## 2. File structure at a glance

```text
{
  "<CableType>": [            <- e.g. "CDFP", "QSFP"
    {                         <- one cable
      id
      inventory_Path
      eeprom                  (optional)
      presence                (optional)
      interfaces
      uplink {                <- first end of the cable
        inventory_Path
        eeprom                (optional)
        presence { gpio }     (optional)
        association           (optional)
      }
      downlink {              <- second end of the cable
        inventory_Path
        eeprom                (optional)
        presence { gpio }     (optional)
        association           (optional)
      }
    },
    { ...next cable of the same type... }
  ],
  "<AnotherCableType>": [ ... ]
}
```

---

## 3. Template

Copy this template and replace every value in angle brackets (`<...>`). JSON
does not allow comments, so all explanations are in the
[field reference](#4-field-reference) below. Remove any optional block you don't
need (see the [minimal example](#6-minimal-example)).

`<cable path>` is the cable's unique inventory path, for example
`/xyz/openbmc_project/inventory/cables/cdfp0`. The uplink and downlink paths are
always that same path with `/uplink` or `/downlink` appended.

```json
{
  "<CableType>": [
    {
      "id": "<id>",
      "inventory_Path": "<cable path>",
      "eeprom": "<path to cable EEPROM>",
      "presence": {
        "Pin_name": "<GPIO line name>",
        "value": "<GPIO value meaning present>"
      },
      "interfaces": {
        "xyz.openbmc_project.Inventory.Item": {
          "PrettyName": "<human-readable cable name>"
        },
        "xyz.openbmc_project.Inventory.Item.Cable": {
          "CableTypeDescription": "<cable type description>"
        }
      },
      "uplink": {
        "inventory_Path": "<cable path>/uplink",
        "eeprom": "<path to uplink EEPROM>",
        "presence": {
          "gpio": {
            "Pin_name": "<GPIO line name>",
            "value": "<GPIO value meaning present>"
          }
        },
        "association": {
          "itemType": "<D-Bus interface specific to the associated item>",
          "endpointPath": "<inventory path of the associated item>",
          "fType": "<forward association name>",
          "rType": "<reverse association name>"
        }
      },
      "downlink": {
        "inventory_Path": "<cable path>/downlink",
        "eeprom": "<path to downlink EEPROM>",
        "presence": {
          "gpio": {
            "Pin_name": "<GPIO line name>",
            "value": "<GPIO value meaning present>"
          }
        },
        "association": {
          "itemType": "<D-Bus interface specific to the associated item>",
          "endpointPath": "<inventory path of the associated item>",
          "fType": "<forward association name>",
          "rType": "<reverse association name>"
        }
      }
    }
  ]
}
```

---

## 4. Field reference

### 4.1 Top level

| Field         | Type  | Required     | Description                                                                                 |
| ------------- | ----- | ------------ | ------------------------------------------------------------------------------------------- |
| `<CableType>` | array | At least one | Key is the cable type name (`CDFP`, `QSFP`). Value is a list of cable objects of that type. |

### 4.2 Cable object

| Field            | Type   | Required      | Description                                                                                                                                                                                                                                                 |
| ---------------- | ------ | ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `id`             | string | **Mandatory** | Identifier of the cable, unique within its cable type. By convention, numbered from `"0"`.                                                                                                                                                                  |
| `inventory_Path` | string | **Mandatory** | D-Bus object path where the cable is published. It can be any path, as long as it is **unique across all cables**. The preferred convention is `/xyz/openbmc_project/inventory/cables/<cableType><id>`, e.g. `/xyz/openbmc_project/inventory/cables/cdfp0`. |
| `eeprom`         | string | Optional      | Path to the EEPROM holding VPD for the cable as a whole. Defining it at this level means the VPD belongs to the entire cable, not to one end.                                                                                                               |
| `presence`       | object | Optional      | GPIO used to detect whether the overall cable is present. See [4.4](#44-presence-cable-level). If omitted, no GPIO presence detection is done for the cable.                                                                                                |
| `interfaces`     | object | **Mandatory** | D-Bus interfaces and properties published on the cable's object path by default. See [4.3](#43-interfaces).                                                                                                                                                 |
| `uplink`         | object | **Mandatory** | First end of the cable. A cable always has two ends. See [4.5](#45-uplink-and-downlink).                                                                                                                                                                    |
| `downlink`       | object | **Mandatory** | Second end of the cable. See [4.5](#45-uplink-and-downlink).                                                                                                                                                                                                |

### 4.3 `interfaces`

A map of D-Bus interface name to a map of property name and value. Every
interface listed here is published on the cable's object path with the given
property values.

| Interface                                  | Property               | Type   | Description                                                                                    |
| ------------------------------------------ | ---------------------- | ------ | ---------------------------------------------------------------------------------------------- |
| `xyz.openbmc_project.Inventory.Item`       | `PrettyName`           | string | Human-readable name of the cable, shown in tools and Redfish, e.g. `"Node 0 CDFP Left cable"`. |
| `xyz.openbmc_project.Inventory.Item.Cable` | `CableTypeDescription` | string | Description of the cable type, e.g. `"CDFP0"`.                                                 |

### 4.4 `presence` (cable level)

At the cable level, the GPIO fields sit **directly** inside `presence`:

```json
"presence": {
    "Pin_name": "cdfp0-present",
    "value": "0"
}
```

| Field      | Type   | Required (if `presence` is used) | Description                                                                           |
| ---------- | ------ | -------------------------------- | ------------------------------------------------------------------------------------- |
| `Pin_name` | string | Yes                              | GPIO line name, as defined in the device tree (`gpio-line-names`).                    |
| `value`    | string | Yes                              | GPIO value that means **present**. For example `"0"` for an active-low presence line. |

### 4.5 `uplink` and `downlink`

Both ends use exactly the same fields.

| Field            | Type   | Required      | Description                                                                                                                                                                                                                                 |
| ---------------- | ------ | ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `inventory_Path` | string | **Mandatory** | D-Bus object path for this end. It **must** be the cable's `inventory_Path` with `/uplink` or `/downlink` appended. For a cable at `/xyz/openbmc_project/inventory/cables/cdfp0`, the ends are `.../cdfp0/uplink` and `.../cdfp0/downlink`. |
| `eeprom`         | string | Optional      | Path to an EEPROM holding VPD for **this end only**.                                                                                                                                                                                        |
| `presence`       | object | Optional      | GPIO used to detect whether this specific end is present. See [4.6](#46-presence-uplink--downlink-level).                                                                                                                                   |
| `association`    | object | Optional      | Links this end to another inventory item (port, connector, chassis). See [4.7](#47-association).                                                                                                                                            |

### 4.6 `presence` (uplink / downlink level)

At the uplink and downlink level, the GPIO fields are **nested inside a `gpio`
object**. This is different from the cable level (see
[5. Rules](#5-rules-and-conventions)).

```json
"presence": {
    "gpio": {
        "Pin_name": "cdfp0-uplink-present",
        "value": "0"
    }
}
```

| Field           | Type   | Required (if `presence` is used) | Description                                    |
| --------------- | ------ | -------------------------------- | ---------------------------------------------- |
| `gpio.Pin_name` | string | Yes                              | GPIO line name, as defined in the device tree. |
| `gpio.value`    | string | Yes                              | GPIO value that means **present**.             |

### 4.7 `association`

Defines which inventory item this end of the cable is connected to. Use it only
if the associated item is itself published as an inventory object.

| Field          | Type   | Required (if `association` is used) | Description                                                                                                                                                                         |
| -------------- | ------ | ----------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `itemType`     | string | Yes                                 | D-Bus interface implemented by the associated item, e.g. `xyz.openbmc_project.Inventory.Item.Connector`. Must be an endpoint type allowed by the `Item.Cable` interface definition. |
| `endpointPath` | string | Yes                                 | Inventory object path of the associated item, e.g. `/xyz/openbmc_project/inventory/system/chassis/motherboard/cdfp_connector0`.                                                     |
| `fType`        | string | Yes                                 | Forward association name (from the cable end to the item), as defined in the `Item.Cable` interface, e.g. `upstream_connector`.                                                     |
| `rType`        | string | Yes                                 | Reverse association name (from the item back to the cable end), as defined in the `Item.Cable` interface, e.g. `upstream_cable`.                                                    |

> The association names (`fType` / `rType`) and the allowed `itemType` values
> come from `xyz/openbmc_project/Inventory/Item/Cable.interface.yaml` in
> phosphor-dbus-interfaces. Check that file in your build for the exact names,
> since they must match.

---

## 5. Rules and conventions

1. **Valid JSON only.** No comments, no trailing commas, and a comma between
   every element in an array (`[ {...}, {...} ]`).
2. **`id` must be unique** within a cable type. The same `id` may be reused
   across different cable types (`CDFP` id `0` and `QSFP` id `0` are both fine).
3. **Cable paths must be unique.** No two cables may share an `inventory_Path`.
   The preferred form is
   `/xyz/openbmc_project/inventory/cables/<cableType><id>`.
4. **End paths extend the cable path.** The uplink and downlink `inventory_Path`
   must be the cable's `inventory_Path` followed by `/uplink` and `/downlink`.
5. **Uplink and downlink are always present.** Even if an end has no EEPROM,
   presence GPIO or association, it still needs its `inventory_Path`.
6. **Presence format differs by level.**
   - Cable level: `"presence": { "Pin_name": ..., "value": ... }`
   - Uplink/downlink level:
     `"presence": { "gpio": { "Pin_name": ..., "value": ... } }`
7. **All values are strings**, including `id` and the GPIO `value` (`"0"`, not
   `0`).
8. **Omit optional blocks entirely** when not needed. Don't leave them empty or
   fill them with placeholders.
9. **GPIO line names** must match the names in the device tree exactly
   (case-sensitive).

---

## 6. Minimal example

Only the mandatory fields, for a cable with no VPD, no presence GPIO and no
associations:

```json
{
  "CDFP": [
    {
      "id": "1",
      "inventory_Path": "/xyz/openbmc_project/inventory/cables/cdfp1",
      "interfaces": {
        "xyz.openbmc_project.Inventory.Item": {
          "PrettyName": "Node 0 CDFP Right cable"
        },
        "xyz.openbmc_project.Inventory.Item.Cable": {
          "CableTypeDescription": "CDFP1"
        }
      },
      "uplink": {
        "inventory_Path": "/xyz/openbmc_project/inventory/cables/cdfp1/uplink"
      },
      "downlink": {
        "inventory_Path": "/xyz/openbmc_project/inventory/cables/cdfp1/downlink"
      }
    }
  ]
}
```

---

## 7. Validation checklist

Before committing the JSON, check that:

- [ ] The file is valid JSON. Run `jq . <file>.json` or
      `python3 -m json.tool <file>.json`; either prints an error and the line
      number if something is wrong.
- [ ] Every cable has `id`, `inventory_Path`, `uplink` and `downlink`.
- [ ] Every `uplink` and `downlink` has its own `inventory_Path`.
- [ ] `id`s are unique within each cable type.
- [ ] Every cable `inventory_Path` is unique across the whole file.
- [ ] Every uplink and downlink path is its cable's path followed by `/uplink`
      or `/downlink`.
- [ ] Cable-level `presence` has `Pin_name` and `value` directly; end-level
      `presence` has them inside `gpio`.
- [ ] GPIO line names exist in the device tree.
- [ ] Every `association` has all four fields, and `fType` / `rType` match the
      names in `Item.Cable`.
- [ ] No placeholders (`<...>`) remain.
