import { loadFeature, defineFeature } from 'jest-cucumber';
import * as React from 'react';
import { render, fireEvent, RenderResult } from '@testing-library/preact';
import configureStore, { MockStore } from 'redux-mock-store';
import { MuiThemeProvider } from '@material-ui/core/styles';
import { Provider } from 'react-redux';
import { AlarmModal } from '../../src/modules/controllers';
import { darkTheme } from '../../src/styles/customTheme';
import { updateCommittedState } from '../../src/store/controller/actions';
import { ALARM_LIMITS, commitAction } from '../../src/store/controller/types';

const feature = loadFeature('tests/features/alarm-modal.feature');

defineFeature(feature, (test) => {
  const mockStore = configureStore();
  let wrapper: RenderResult;
  let store: MockStore;
  let commitChange: (min: number, max: number) => void;
  const clickInterval = 100;

  beforeEach(() => {
    commitChange = jest.fn();
    store = mockStore({
      controller: { alarmLimitsRequest: { rr: { lower: 0, upper: clickInterval } } },
    });
    wrapper = render(
      <Provider store={store}>
        <MuiThemeProvider theme={darkTheme}>
          <AlarmModal
            label="dummy"
            units=""
            committedMin={0}
            committedMax={100}
            requestCommitRange={commitChange}
            stateKey="rr"
          />
        </MuiThemeProvider>
      </Provider>,
    );
    fireEvent.click(wrapper.getByText(/alarm/i, { exact: false }));
  });

  test('To check if Alarm increment is working', ({ given, when, then }) => {
    let valueClickerMin: HTMLElement;

    given(/^I mount Alarm modal component$/, () => {
      valueClickerMin = wrapper.getAllByRole('heading')[2];
      expect(wrapper).toBeDefined();
      expect(valueClickerMin).toBeDefined();
    });

    when(/^I click on increment button$/, async () => {
      const buttons = wrapper.getAllByRole('button');
      expect(valueClickerMin.textContent).toBe('0');
      await fireEvent.click(buttons[0]);
      await new Promise((r) => setTimeout(r, clickInterval));
    });

    then(/^I should get the result as incremented value$/, () => {
      expect(valueClickerMin.textContent).toBe('1');
    });
  });

  test('To check if Alarm decrement is working', ({ given, when, then }) => {
    let valueClickerMax: HTMLElement;

    given(/^I mount Alarm modal component$/, () => {
      valueClickerMax = wrapper.getAllByRole('heading')[3];
      expect(wrapper).toBeDefined();
      expect(valueClickerMax).toBeDefined();
    });

    when(/^I click on decrement button$/, async () => {
      const buttons = wrapper.getAllByRole('button');
      expect(valueClickerMax.textContent).toBe('100');
      await fireEvent.click(buttons[3]);
      await new Promise((r) => setTimeout(r, clickInterval));
    });

    then(/^I should get the result as decremented value$/, () => {
      expect(valueClickerMax.textContent).toBe('99');
    });
  });

  test('To check if confirm button is working', ({ given, when, then }) => {
    given(/^I set alarm limit values$/, async () => {
      const buttons = wrapper.getAllByRole('button');
      expect(wrapper).toBeDefined();
      await fireEvent.click(buttons[0]);
      await new Promise((r) => setTimeout(r, clickInterval));
      await fireEvent.click(buttons[3]);
      await new Promise((r) => setTimeout(r, clickInterval));
    });

    when(/^I click on confirm button$/, async () => {
      const confirmButton = wrapper.getByRole('button', { name: /confirm/i });
      await fireEvent.click(confirmButton);
    });

    then(/^I should get confirm action called with values I have set$/, () => {
      // Since on every value change CommitChange is being triggered
      expect(commitChange).toHaveBeenCalledTimes(4);
      expect(commitChange).toBeCalledWith(1, 99);
    });
  });

  test('To check if alarm values are stored in redux', ({ given, when, then }) => {
    let actionCommitted: commitAction;

    given(/^I set alarm limit values$/, async () => {
      actionCommitted = updateCommittedState(ALARM_LIMITS, { rr: { lower: 10, upper: 20 } });
    });

    when(/^I commit changes to redux action$/, async () => {
      store.dispatch(actionCommitted);
    });

    then(/^I should get action values verified with triggered one's$/, () => {
      const actions = store.getActions();
      const expectedPayload = {
        type: '@controller/ALARM_LIMITS_COMMITTED',
        update: { rr: { lower: 10, upper: 20 } },
      };
      expect(actions).toEqual([expectedPayload]);
    });
  });
});
