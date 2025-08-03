import React from 'react';
import ReactDOM from 'react-dom/client';
import 'bootstrap/dist/css/bootstrap.css';

import {
  createHashRouter,
  createRoutesFromElements,
  Route,
  RouterProvider,
} from "react-router-dom";

import './index.css';
import {App} from './App';
import {ErrorPage} from './ErrorPage';
import {HomePage} from './HomePage';
import {PatternsPage} from './Patterns';
import {SetupPage} from './SetupPage';
import { Spinner } from 'react-bootstrap';
import { AddPatternPage } from './AddPattern';
import EditPatternPage from './EditPattern';



import { useAppStatus } from './hooks/useAppStatus';

export function AppModeSwitcher() {
  const appStatus = useAppStatus();

  if(appStatus == null)
    return (<Spinner animation="grow" variant="warning"  style={{ width: "8rem", height: "8rem" }} />);

  if(appStatus.apMode)
    // Limited UI for connecting to WiFi
    return (<HomePage />);

  const router = createHashRouter(
    createRoutesFromElements(
      <Route path="/" element={<App/>}
      errorElement={<ErrorPage/>}>
      <Route index element={<HomePage />} />
      <Route path="/patterns" element={<PatternsPage />} />
      <Route path="/patterns/add" element={<AddPatternPage />} />
      <Route path="/patterns/edit/:id" element={<EditPatternPage />} />
      <Route path="/setup" element={<SetupPage />} />
    </Route>
    ));
    
  return (<RouterProvider router={router} />);
}

const root = ReactDOM.createRoot(
  document.getElementById('root') as HTMLElement
);
root.render(
  <React.StrictMode>
    <AppModeSwitcher />
  </React.StrictMode>
);

