import { Grid, Typography } from '@material-ui/core';
import { makeStyles, Theme, useTheme } from '@material-ui/core/styles';
import React, { RefObject, useCallback, useEffect, useRef } from 'react';
import { shallowEqual, useDispatch, useSelector } from 'react-redux';
import { updateCommittedState } from '../../store/controller/actions';
import { VentilationMode } from '../../store/controller/proto/mcu_pb';
import {
  getParametersRequestMode,
  getParametersRequestStandby,
} from '../../store/controller/selectors';
import { PARAMETER_STANDBY } from '../../store/controller/types';
import { setActiveRotaryReference } from '../app/Service';
import ValueClicker from '../controllers/ValueController';
import ModeBanner from '../displays/ModeBanner';
import { LMIN, PERCENT } from '../info/units';
import {
  FIO2_REFERENCE_KEY,
  PEEP_REFERENCE_KEY,
  RR_REFERENCE_KEY,
  TV_REFERENCE_KEY,
  FLOW_REFERENCE_KEY,
} from '../settings/containers/constants';
import { useRotaryReference } from '../utils/useRotaryReference';

const useStyles = makeStyles((theme: Theme) => ({
  root: {
    justifyContent: 'space-between',
    height: '100%',
    flexWrap: 'nowrap',
    // border: '1px solid red',
  },
  topPanel: {
    flexWrap: 'nowrap',
    borderRadius: theme.panel.borderRadius,
    marginBottom: '15px',
    justifyContent: 'space-between',
    alignItems: 'stretch',
    backgroundColor: theme.palette.background.paper,
  },
  standby: {
    padding: theme.spacing(2),
    borderRight: `2px dashed ${theme.palette.background.default}`,
  },
  padding: {
    padding: theme.spacing(2),
    // border: '1px solid red'
  },
  paddingTop: {
    paddingTop: theme.spacing(3),
  },
  middleContainer: {
    flexWrap: 'nowrap',
    width: '100%',
    marginBottom: '15px',
    // border: '1px solid teal',
  },
  middleLeftPanel: {
    borderRadius: theme.panel.borderRadius,
    marginRight: theme.spacing(2),
    backgroundColor: theme.palette.background.paper,
  },
  middleRightPanel: {
    borderRadius: theme.panel.borderRadius,
    backgroundColor: theme.palette.background.paper,
  },
  bottomBorder: {
    borderBottom: `2px dashed ${theme.palette.background.default}`,
  },
  rightBorder: {
    borderRight: `2px dashed ${theme.palette.background.default}`,
  },
}));

// enum PatientSex {
//   MALE,
//   FEMALE,
// }
// enum PatientAge {
//   ADULT,
//   PEDIATRIC,
// }

const SettableParameters = (): JSX.Element => {
  const classes = useStyles();
  const [elRefs] = React.useState<Record<string, RefObject<HTMLDivElement>>>({
    [PEEP_REFERENCE_KEY]: useRef(null),
    [RR_REFERENCE_KEY]: useRef(null),
    [FIO2_REFERENCE_KEY]: useRef(null),
    [TV_REFERENCE_KEY]: useRef(null),
    [FLOW_REFERENCE_KEY]: useRef(null),
  });
  const parameterStandby = useSelector(getParametersRequestStandby, shallowEqual);

  const [PEEP, setPEEP] = React.useState(parameterStandby.peep);
  const [Flow, setFlow] = React.useState(parameterStandby.flow);
  const [RR, setRR] = React.useState(parameterStandby.rr);
  const [TV, setTV] = React.useState(parameterStandby.vt);
  const [FiO2, setFiO2] = React.useState(parameterStandby.fio2);
  const dispatch = useDispatch();
  const theme = useTheme();
  const { initRefListener } = useRotaryReference(theme);

  const initParameterUpdate = useCallback(() => {
    dispatch(
      updateCommittedState(PARAMETER_STANDBY, {
        peep: PEEP,
        vt: TV,
        rr: RR,
        fio2: FiO2,
        flow: Flow,
      }),
    );
  }, [PEEP, Flow, RR, TV, FiO2, dispatch]);

  useEffect(() => {
    initParameterUpdate();
    initRefListener(elRefs);
  }, [initParameterUpdate, initRefListener, elRefs]);

  const ventilationMode = useSelector(getParametersRequestMode);
  switch (ventilationMode) {
    case VentilationMode.hfnc:
      return (
        <Grid container item xs={8} direction="column" className={classes.middleRightPanel}>
          <Grid container item xs direction="row" className={classes.bottomBorder}>
            <Grid item xs className={classes.rightBorder}>
              <ValueClicker
                reference={elRefs[FIO2_REFERENCE_KEY]}
                referenceKey={FIO2_REFERENCE_KEY}
                label="FiO2"
                units={PERCENT}
                value={FiO2}
                onClick={setFiO2}
              />
            </Grid>
            <Grid item xs>
              <ValueClicker
                reference={elRefs[FLOW_REFERENCE_KEY]}
                referenceKey={FLOW_REFERENCE_KEY}
                label="Flow"
                units={LMIN}
                value={Flow}
                onClick={setFlow}
              />
            </Grid>
          </Grid>
          <Grid container item xs direction="row">
            <Grid item xs className={classes.rightBorder} />
            <Grid item xs />
          </Grid>
        </Grid>
      );
    case VentilationMode.pc_ac:
    case VentilationMode.vc_ac:
    case VentilationMode.niv_pc:
    case VentilationMode.niv_ps:
    case VentilationMode.psv:
    default:
      return (
        <Grid container item xs={8} direction="column" className={classes.middleRightPanel}>
          <Grid container item xs direction="row" className={classes.bottomBorder}>
            <Grid item xs className={classes.rightBorder}>
              <ValueClicker
                reference={elRefs[PEEP_REFERENCE_KEY]}
                referenceKey={PEEP_REFERENCE_KEY}
                label="PEEP"
                units="cm H2O"
                value={PEEP}
                onClick={setPEEP}
              />
            </Grid>
            <Grid item xs>
              <ValueClicker
                reference={elRefs[RR_REFERENCE_KEY]}
                referenceKey={RR_REFERENCE_KEY}
                label="RR"
                units="cm H2O"
                value={RR}
                onClick={setRR}
              />
            </Grid>
          </Grid>
          <Grid container item xs direction="row">
            <Grid item xs className={classes.rightBorder}>
              <ValueClicker
                reference={elRefs[FIO2_REFERENCE_KEY]}
                referenceKey={FIO2_REFERENCE_KEY}
                label="FiO2"
                units="%"
                value={FiO2}
                onClick={setFiO2}
              />
            </Grid>
            <Grid item xs>
              <ValueClicker
                reference={elRefs[TV_REFERENCE_KEY]}
                referenceKey={TV_REFERENCE_KEY}
                label="TV"
                units="mL"
                value={TV}
                max={500}
                onClick={setTV}
              />
            </Grid>
          </Grid>
        </Grid>
      );
  }
};

export const QuickStartPage = (): JSX.Element => {
  const classes = useStyles();
  // const [patientSex, setPatientSex] = React.useState(PatientSex.MALE);
  // const [patientAge, setPatientAge] = React.useState(PatientAge.ADULT);
  // const [patientHeight, setPatientHeight] = React.useState(62);
  // const [patientCircuitTestDate] = React.useState(new Date());
  // const [preUseCheckDate] = React.useState(new Date());

  const OnClickPage = () => {
    setActiveRotaryReference(null);
  };

  return (
    <Grid container direction="column" className={classes.root} onClick={OnClickPage}>
      <Grid container item className={classes.topPanel}>
        <Grid item xs className={classes.standby}>
          <Typography variant="h3">Standby</Typography>
          <Typography variant="body1">Patient Not Ventilated</Typography>
        </Grid>
        <Grid item xs className={`${classes.standby} ${classes.padding}`}>
          {/* <Typography variant="h6">Age:</Typography>
          <Grid item xs className={classes.paddingTop}>
            <ToggleValue
              toggleBetween={[
                { label: 'Adult', value: PatientAge.ADULT },
                { label: 'Pediatric', value: PatientAge.PEDIATRIC },
              ]}
              onToggle={(selected: number) => setPatientAge(selected as PatientAge)}
              selected={patientAge}
            />
          </Grid> */}
        </Grid>
        <Grid item xs className={`${classes.standby} ${classes.padding}`}>
          {/* <Typography variant="h6">Sex:</Typography>
          <Grid item xs className={classes.paddingTop}>
            <ToggleValue
              toggleBetween={[
                { label: 'Male', value: PatientSex.MALE },
                { label: 'Female', value: PatientSex.FEMALE },
              ]}
              onToggle={(selected: number) => setPatientSex(selected as PatientSex)}
              selected={patientSex}
            />
          </Grid> */}
        </Grid>
        <Grid item xs={3}>
          {/* <ValueClicker
            label="Height"
            units="i"
            onClick={setPatientHeight}
            value={patientHeight}
            min={6}
            max={120}
          /> */}
        </Grid>
      </Grid>
      <Grid container item xs className={classes.middleContainer}>
        <Grid container item xs direction="column" className={classes.middleLeftPanel}>
          <Grid item xs className={classes.bottomBorder}>
            {/* <TestTool label="Pre-Use Check" isSuccess={true} timestamp={preUseCheckDate} /> */}
          </Grid>
          <Grid item xs>
            {/* <TestTool
              label="Patient Circuit Test"
              isSuccess={true}
              timestamp={patientCircuitTestDate}
            /> */}
          </Grid>
        </Grid>
        <SettableParameters />
      </Grid>
      <Grid>
        <ModeBanner bannerType="normal" />
      </Grid>
    </Grid>
  );
};

export default QuickStartPage;
