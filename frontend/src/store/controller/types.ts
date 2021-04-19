import {
  SensorMeasurements,
  CycleMeasurements,
  Parameters,
  ParametersRequest,
  AlarmLimits,
  AlarmLimitsRequest,
  ActiveLogEvents,
  ExpectedLogEvent,
  NextLogEvents,
  BatteryPower,
  ScreenStatus,
  AlarmMuteRequest,
  AlarmMute,
} from './proto/mcu_pb';
import { SystemSettingRequest, FrontendDisplaySetting, RotaryEncoder } from './proto/frontend_pb';

// Action Types

export const STATE_UPDATED = '@controller/STATE_UPDATED';
export const PARAMETER_COMMITTED = '@controller/PARAMETER_COMMITTED';
export const ALARM_LIMITS = 'ALARM_LIMITS';
export const EXPECTED_LOG_EVENT_ID = 'EXPECTED_LOG_EVENT_ID';
export const ALARM_LIMITS_STANDBY = 'ALARM_LIMITS_STANDBY';
export const PARAMETER_STANDBY = 'PARAMETERS_STANDBY';
export const FRONTEND_DISPLAY_SETTINGS = 'FRONTEND_DISPLAY_SETTINGS';
export const SYSTEM_SETTINGS = 'SYSTEM_SETTINGS';
export const ALARM_MUTE = 'ALARM_MUTE';
export const HEARTBEAT_BACKEND = '@controller/HEARTBEAT_BACKEND';
export const BACKEND_CONNECTION_LOST = '@controller/BACKEND_CONNECTION_LOST';
export const BACKEND_CONNECTION_LOST_CODE = 101;

// Rotary encoder overriden params

export interface RotaryEncoderParameter {
  step: number;
  lastStepChange: number;
  buttonPressed: boolean;
  lastButtonDown: number;
  lastButtonUp: number;
  stepDiff: number;
}

// Protocol Buffers

export type PBMessage =
  // mcu_pb
  | SensorMeasurements
  | CycleMeasurements
  | Parameters
  | ParametersRequest
  | AlarmLimits
  | AlarmLimitsRequest
  | ExpectedLogEvent
  | NextLogEvents
  | ActiveLogEvents
  | AlarmMuteRequest
  | AlarmMute
  // frontend_pb
  | BatteryPower
  | ScreenStatus
  | SystemSettingRequest
  | FrontendDisplaySetting
  | RotaryEncoder;

export type PBMessageType =
  // mcu_pb
  | typeof SensorMeasurements
  | typeof CycleMeasurements
  | typeof Parameters
  | typeof ParametersRequest
  | typeof AlarmLimits
  | typeof AlarmLimitsRequest
  | typeof ExpectedLogEvent
  | typeof NextLogEvents
  | typeof ActiveLogEvents
  | typeof AlarmMuteRequest
  | typeof AlarmMute
  // frontend_pb
  | typeof BatteryPower
  | typeof ScreenStatus
  | typeof SystemSettingRequest
  | typeof FrontendDisplaySetting
  | typeof RotaryEncoder;

export enum MessageType {
  // mcu_pb
  SensorMeasurements = 2,
  CycleMeasurements = 3,
  Parameters = 4,
  ParametersRequest = 5,
  AlarmLimits = 6,
  AlarmLimitsRequest = 7,
  ExpectedLogEvent = 8,
  NextLogEvents = 9,
  ActiveLogEvents = 10,
  AlarmMuteRequest = 11,
  AlarmMute = 12,
  // frontend_pb
  BatteryPower = 64,
  ScreenStatus = 65,
  RotaryEncoder = 128,
  SystemSettingRequest = 129,
  FrontendDisplaySetting = 130,
}

// States

export interface WaveformPoint {
  date: number;
  value: number;
}

export interface WaveformHistory {
  waveformOld: {
    full: WaveformPoint[];
  };
  waveformNew: {
    full: WaveformPoint[];
    buffer: WaveformPoint[];
    segmented: WaveformPoint[][];
  };
  waveformNewStart: number;
}

export interface PVPoint {
  pressure: number;
  volume: number;
}

export interface PVHistory {
  loop: PVPoint[];
  loopOrigin: PVPoint;
  cycle: number;
}

export interface SmoothingData {
  raw: number;
  average: number;
  converged: number;
  smoothed: number;
  time?: number;
  convergenceStartTime?: number;
  changeStartTime?: number;
}

export interface ControllerStates {
  // Message states from mcu_pb
  sensorMeasurements: SensorMeasurements;
  cycleMeasurements: CycleMeasurements;
  parameters: Parameters;
  parametersRequest: ParametersRequest;
  parametersRequestStandby: { parameters: ParametersRequest };
  alarmLimits: AlarmLimits;
  alarmLimitsRequest: AlarmLimitsRequest;
  alarmLimitsRequestStandby: { alarmLimits: AlarmLimitsRequest };
  expectedLogEvent: ExpectedLogEvent;
  nextLogEvents: NextLogEvents;
  heartbeatBackend: { time: Date };
  activeLogEvents: ActiveLogEvents;
  alarmMuteRequest: AlarmMuteRequest;
  alarmMute: AlarmMute;
  systemSettingRequest: SystemSettingRequest;
  frontendDisplaySetting: FrontendDisplaySetting;
  batteryPower: BatteryPower;
  screenStatus: ScreenStatus;

  // Message states from frontend_pb
  rotaryEncoder: RotaryEncoderParameter;

  // Derived states
  smoothedMeasurements: {
    fio2: SmoothingData;
    flow: SmoothingData;
    spo2: SmoothingData;
    hr: SmoothingData;
  };
  waveformHistoryPaw: WaveformHistory;
  waveformHistoryFlow: WaveformHistory;
  waveformHistoryVolume: WaveformHistory;
  pvHistory: PVHistory;
}

export const MessageClass = new Map<MessageType, PBMessageType>([
  // mcu_pb
  [MessageType.SensorMeasurements, SensorMeasurements],
  [MessageType.CycleMeasurements, CycleMeasurements],
  [MessageType.Parameters, Parameters],
  [MessageType.ParametersRequest, ParametersRequest],
  [MessageType.AlarmLimits, AlarmLimits],
  [MessageType.AlarmLimitsRequest, AlarmLimitsRequest],
  [MessageType.ExpectedLogEvent, ExpectedLogEvent],
  [MessageType.NextLogEvents, NextLogEvents],
  [MessageType.ActiveLogEvents, ActiveLogEvents],
  [MessageType.AlarmMuteRequest, AlarmMuteRequest],
  [MessageType.AlarmMute, AlarmMute],
  // frontend_pb
  [MessageType.BatteryPower, BatteryPower],
  [MessageType.ScreenStatus, ScreenStatus],
  [MessageType.SystemSettingRequest, SystemSettingRequest],
  [MessageType.FrontendDisplaySetting, FrontendDisplaySetting],
  [MessageType.RotaryEncoder, RotaryEncoder],
]);

export const MessageTypes = new Map<PBMessageType, MessageType>([
  // mcu_pb
  [SensorMeasurements, MessageType.SensorMeasurements],
  [CycleMeasurements, MessageType.CycleMeasurements],
  [Parameters, MessageType.Parameters],
  [ParametersRequest, MessageType.ParametersRequest],
  [AlarmLimits, MessageType.AlarmLimits],
  [AlarmLimitsRequest, MessageType.AlarmLimitsRequest],
  [ExpectedLogEvent, MessageType.ExpectedLogEvent],
  [NextLogEvents, MessageType.NextLogEvents],
  [ActiveLogEvents, MessageType.ActiveLogEvents],
  [AlarmMuteRequest, MessageType.AlarmMuteRequest],
  [AlarmMute, MessageType.AlarmMute],
  // frontend_pb
  [BatteryPower, MessageType.BatteryPower],
  [ScreenStatus, MessageType.ScreenStatus],
  [SystemSettingRequest, MessageType.SystemSettingRequest],
  [FrontendDisplaySetting, MessageType.FrontendDisplaySetting],
  [RotaryEncoder, MessageType.RotaryEncoder],
]);

// State Update Actions

interface StateUpdatedAction {
  type: typeof STATE_UPDATED;
  messageType: MessageType;
  state: PBMessage;
}

export type StateUpdateAction = StateUpdatedAction;

// State Update Actions

export interface commitAction {
  type: string;
  update: Record<string, unknown>;
}

// Parameter Commit Actions

interface ParameterCommittedAction {
  type: typeof PARAMETER_COMMITTED;
  update: Record<string, unknown>;
}

export type ParameterCommitAction = ParameterCommittedAction;
