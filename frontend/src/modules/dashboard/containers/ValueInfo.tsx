import { Grid, makeStyles, Theme, Typography } from '@material-ui/core';
import React, { useEffect, useState } from 'react';
import { shallowEqual, useSelector } from 'react-redux';
import { getAlarmLimitsRequest } from '../../../store/controller/selectors';
import { setMultiPopupOpen } from '../../app/Service';
import { AlarmModal } from '../../controllers';
import { SelectorType, ValueSelectorDisplay } from '../../displays/ValueSelectorDisplay';
import { Range } from '../../../store/controller/proto/mcu_pb';

const useStyles = makeStyles((theme: Theme) => ({
  rootParent: {
    flexDirection: 'initial',
    height: '100%',
  },
  root: {
    width: '100%',
    // border: '1px solid red'
    backgroundColor: '#010010',
    borderRadius: '8px',
    padding: '10px',
    height: '100%',
  },
  displayContainer: {
    padding: 0,
    // border: '1px solid orange',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    width: '100%',
  },
  liveContainer: {
    justifyContent: 'flex-end',
    alignItems: 'center',
    height: '100%',
    display: 'block',
    textAlign: 'right',
    // border: '1px solid blue'
  },
  liveBox: {
    textAlign: 'center',
    width: '100%',
    fontSize: 14,
    borderRadius: 5,
    border: `2px solid ${theme.palette.primary.main}`,
  },
  valueContainer: {
    justifyContent: 'flex-start',
    alignItems: 'center',
    flexGrow: 1,
    // border: '1px solid yellow',
  },
  valueLabel: {
    fontSize: '3.5rem',
    lineHeight: '1',
    // border: '1px solid red',
  },
  unitsLabel: {
    // paddingLeft: theme.spacing(1),
    // paddingTop: theme.spacing(4),
    opacity: 0.8,
    // border: '1px solid red'
  },
  valuesPanel: {
    padding: '15px',
    display: 'grid',
    gridTemplateColumns: '135px 1fr',
    gridGap: '10px',
    gridTemplateAreas: `'gridAreavalues1 gridAreavalues2'
    'gridAreavalues1 gridAreavalues3'`,
  },
  mainWithSubcontainer: {
    gridTemplateAreas: `'gridAreavalues1 gridAreavalues2'
    'gridAreavalues1 gridAreavalues3'`,
  },
  mainContainer: {
    gridTemplateAreas: `'gridAreavalues1 gridAreavalues1'
    'gridAreavalues1 gridAreavalues1'`,
  },
  gridAreavalues1: {
    gridArea: 'gridAreavalues1',
  },
  gridAreavalues2: {
    gridArea: 'gridAreavalues2',
  },
  gridAreavalues3: {
    gridArea: 'gridAreavalues3',
  },

  gridRoot: {
    width: '100%',
    // border: '1px solid red'
    backgroundColor: '#010010',
    borderRadius: '8px',
    padding: '5px 10px',
    height: '100%',
  },
  gridLiveContainer: {
    // display: 'flex',
    justifyContent: 'flex-end',
    height: '100%',
    // border: '1px solid blue'
    textAlign: 'right',
  },

  gridValueLabel: {
    fontSize: '2rem',
    lineHeight: '1',
    // border: '1px solid red',
  },
  gridUnitsLabel: {
    // paddingLeft: theme.spacing(1),
    paddingTop: theme.spacing(3),
    opacity: 0.8,
    fontSize: '12px',
    // border: '1px solid red'
  },
  whiteFont: {
    color: '#fff',
  },
}));

export interface ValueInfoProps {
  mainContainer: Props;
  subContainer1?: Props;
  subContainer2?: Props;
}

export interface Props {
  selector: SelectorType;
  label: string;
  stateKey: string;
  units?: string;
  isLive?: boolean;
  isMain?: boolean;
  decimal?: number;
}

export const ClickHandler = (
  singleClickAction: () => void,
  doubleClickAction: () => void,
): (() => void) => {
  const [click, setClick] = useState(0);

  useEffect(() => {
    const timer = setTimeout(() => {
      if (click === 1) singleClickAction();
      setClick(0);
    }, 100);
    if (click === 2) doubleClickAction();

    return () => {
      clearTimeout(timer);
    };
  }, [click, singleClickAction, doubleClickAction]);

  return () => {
    setClick((prev) => prev + 1);
  };
};

const ControlValuesDisplay = ({
  selector,
  label,
  stateKey,
  units = '',
  isMain = false,
  decimal,
}: Props): JSX.Element => {
  const classes = useStyles();
  const [open, setOpen] = useState(false);
  const alarmLimits = useSelector(getAlarmLimitsRequest, shallowEqual) as Record<string, Range>;
  const onClick = () => {
    // setOpen(true);
    if (stateKey) {
      setMultiPopupOpen(true, stateKey);
    }
  };
  const handleClick = ClickHandler(onClick, () => {
    return false;
  });
  const updateModalStatus = (status: boolean) => {
    setOpen(status);
  };
  return (
    <div
      style={{ outline: 'none', height: '100%' }}
      role="button"
      onKeyDown={() => null}
      onClick={handleClick}
      tabIndex={0}
    >
      <Grid container direction="column" className={classes.rootParent}>
        <Grid item xs style={{ width: '100%', height: '100%' }}>
          <Grid
            container
            direction="column"
            className={classes.root}
            style={isMain ? { width: '50%', margin: '0 auto' } : {}}
          >
            <Grid
              container
              item
              direction="row"
              justify="space-between"
              alignItems="center"
              wrap="nowrap"
            >
              <Grid item xs style={{ width: '100%' }}>
                <Typography className={classes.whiteFont} style={{ fontSize: '16px' }}>
                  {label}
                </Typography>
              </Grid>
              {stateKey && (
                <Grid container item xs={3} className={classes.liveContainer}>
                  <Typography className={classes.whiteFont}>
                    {alarmLimits[stateKey].lower}
                  </Typography>
                  <Typography className={classes.whiteFont}>
                    {alarmLimits[stateKey].upper}
                  </Typography>
                </Grid>
              )}
            </Grid>
            <Grid item xs alignItems="center" className={classes.displayContainer}>
              <Grid>
                <Typography
                  align="center"
                  variant="h2"
                  className={`${classes.valueLabel} ${classes.whiteFont}`}
                >
                  <ValueSelectorDisplay decimal={decimal} selector={selector} />
                </Typography>
                {units !== '' && (
                  <Typography
                    align="center"
                    variant="body1"
                    className={`${classes.unitsLabel} ${classes.whiteFont}`}
                  >
                    {units}
                  </Typography>
                )}
              </Grid>
            </Grid>
          </Grid>
        </Grid>
        {stateKey && (
          <AlarmModal
            updateModalStatus={updateModalStatus}
            openModal={open}
            disableAlarmButton={true}
            label={label}
            units={units}
            stateKey={stateKey}
            requestCommitRange={() => null}
          />
        )}
      </Grid>
    </div>
  );
};

const GridControlValuesDisplay = ({
  selector,
  label,
  stateKey,
  units = '',
  decimal,
}: Props): JSX.Element => {
  const classes = useStyles();
  const [open, setOpen] = useState(false);
  const alarmLimits = useSelector(getAlarmLimitsRequest, shallowEqual) as Record<string, Range>;
  const onClick = () => {
    // setOpen(true);
    if (stateKey) {
      setMultiPopupOpen(true, stateKey);
    }
  };
  const handleClick = ClickHandler(onClick, () => {
    return false;
  });
  const updateModalStatus = (status: boolean) => {
    setOpen(status);
  };
  return (
    <div
      style={{ outline: 'none', height: '100%' }}
      role="button"
      onKeyDown={() => null}
      onClick={handleClick}
      tabIndex={0}
    >
      <Grid container direction="column" className={classes.rootParent}>
        <Grid item xs style={{ width: '100%', height: '100%' }}>
          <Grid container direction="column" className={classes.gridRoot}>
            <Grid container item style={{ height: '100%' }}>
              <Grid item xs>
                <Typography className={classes.whiteFont}>{label}</Typography>
              </Grid>

              <Grid container item xs justify="flex-start" alignItems="center" wrap="nowrap">
                <Grid className={classes.displayContainer}>
                  <Typography
                    align="center"
                    variant="h5"
                    className={`${classes.gridValueLabel} ${classes.whiteFont}`}
                  >
                    <ValueSelectorDisplay decimal={decimal} selector={selector} />
                  </Typography>
                  {units !== '' && (
                    <Typography
                      align="center"
                      variant="body1"
                      className={`${classes.gridUnitsLabel} ${classes.whiteFont}`}
                    >
                      {units}
                    </Typography>
                  )}
                </Grid>
              </Grid>
              {stateKey && (
                <Grid item xs className={classes.gridLiveContainer}>
                  <Typography className={classes.whiteFont}>
                    {alarmLimits[stateKey].lower}
                  </Typography>
                  <Typography className={classes.whiteFont}>
                    {alarmLimits[stateKey].upper}
                  </Typography>
                </Grid>
              )}
            </Grid>
          </Grid>
        </Grid>
        {stateKey && (
          <AlarmModal
            updateModalStatus={updateModalStatus}
            openModal={open}
            disableAlarmButton={true}
            label={label}
            units={units}
            stateKey={stateKey}
            requestCommitRange={() => null}
          />
        )}
      </Grid>
    </div>
  );
};
/**
 * Value Info
 *
 * Component for showing information.
 *
 */
const ValueInfo = (props: {
  mainContainer: Props;
  subContainer1?: Props;
  subContainer2?: Props;
}): JSX.Element => {
  const { mainContainer, subContainer1, subContainer2 } = props;
  const classes = useStyles();
  const Render = () => {
    if (mainContainer && !subContainer1 && !subContainer2) {
      return (
        <Grid item xs container className={`${classes.valuesPanel} ${classes.mainContainer}`}>
          <Grid item xs className={classes.gridAreavalues1}>
            <ControlValuesDisplay
              isMain={true}
              stateKey={mainContainer.stateKey}
              selector={mainContainer.selector}
              label={mainContainer.label}
              units={mainContainer.units}
              decimal={mainContainer.decimal || 0}
            />
          </Grid>
        </Grid>
      );
    }
    return (
      <Grid item xs container className={`${classes.valuesPanel} ${classes.mainWithSubcontainer}`}>
        <Grid item xs className={classes.gridAreavalues1}>
          <ControlValuesDisplay
            stateKey={mainContainer.stateKey}
            selector={mainContainer.selector}
            label={mainContainer.label}
            units={mainContainer.units}
            decimal={mainContainer.decimal || 0}
          />
        </Grid>
        <Grid item xs className={classes.gridAreavalues2}>
          {subContainer1 && (
            <GridControlValuesDisplay
              stateKey={subContainer1.stateKey}
              selector={subContainer1.selector}
              label={subContainer1.label}
              units={subContainer1.units}
              decimal={subContainer1.decimal || 0}
            />
          )}
        </Grid>
        <Grid item xs className={classes.gridAreavalues3}>
          {subContainer2 && (
            <GridControlValuesDisplay
              stateKey={subContainer2.stateKey}
              selector={subContainer2.selector}
              label={subContainer2.label}
              units={subContainer2.units}
              decimal={subContainer2.decimal || 0}
            />
          )}
        </Grid>
      </Grid>
    );
  };
  return <Render />;
};

export default ValueInfo;
