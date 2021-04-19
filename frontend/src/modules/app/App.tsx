import { CssBaseline } from '@material-ui/core';
import { MuiThemeProvider } from '@material-ui/core/styles';
import React, { Component } from 'react';
import { IntlProvider } from 'react-intl';
import { connect } from 'react-redux';
import { createStructuredSelector } from 'reselect';
import { initialize } from '../../store/app/actions';
import { getLocale, getMessages } from '../../store/app/selectors';
import { FrontendDisplaySetting, ThemeVariant } from '../../store/controller/proto/frontend_pb';
import { getFrontendDisplaySetting } from '../../store/controller/selectors';
import { StoreState } from '../../store/types';
import { darkTheme, lightTheme } from '../../styles/customTheme';
import Routes from '../navigation/Routes';

const RIGHT_CLICK_BUTTON_KEY = 2;
const TOUCH_SCREEN_BUTTON_KEY = 0;

interface Props {
  initialize: typeof initialize;
  locale: string;
  messages: Record<string, string>;
  displaySetting: FrontendDisplaySetting;
}

class App extends Component<Props> {
  componentDidMount() {
    const { initialize } = this.props;
    initialize();
    // Disable Right click across app
    const disableContext = (event: MouseEvent): boolean => {
      if (event.button === RIGHT_CLICK_BUTTON_KEY || event.button === TOUCH_SCREEN_BUTTON_KEY) {
        event.preventDefault();
        return false;
      }
      return true;
    };
    document.oncontextmenu = disableContext;
  }

  render() {
    const { locale, messages, displaySetting }: Props = this.props;
    const theme = displaySetting.theme === ThemeVariant.dark ? darkTheme : lightTheme;
    return (
      <IntlProvider key={locale} locale={locale} messages={messages}>
        <MuiThemeProvider theme={theme}>
          <CssBaseline />
          <Routes />
        </MuiThemeProvider>
      </IntlProvider>
    );
  }
}

const selector = createStructuredSelector<
  StoreState,
  { locale: string; messages: Record<string, string>; displaySetting: FrontendDisplaySetting }
>({
  locale: getLocale,
  messages: getMessages,
  displaySetting: getFrontendDisplaySetting,
});

const actions = {
  initialize,
};

export default connect(selector, actions)(App);
