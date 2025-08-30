import './HomePage.css';
import { Col, Container, ListGroup, Row } from "react-bootstrap";
import { WifiSetup } from './WifiSetup';
import logo from './logo.svg';
import pico from './pico.svg';
import { useAppStatus } from './hooks/useAppStatus';
import { BasicLightControls } from './BasicLightControls';


export function HomePage() {

  const status = useAppStatus(10_000, true);

  return (
    <> {status == null ? <div>Loading...</div> :
      <div className="p-5 mt-4 mb-4 bg-body-tertiary rounded-3">
        <Container className="py-5">
          <h1 className="display-5 fw-bold">
            <img src={pico} alt="Pico" height={70} className="d-inline-block me-1" />
            <img src={logo} alt="Hatsune Miku" height={70} className="d-inline-block" /></h1>
          <h2 className="fs-4">Mark's Neopixel Animation Controller</h2>
          <p className="col-md-8 fs-5">Control your Neopixel display, and integrate with Home Assistant via Mqtt.</p>
          <p className="col-md-8 fs-5">Use this web interface to configure the controller and edit the lighting scenes.</p>
          <h2 className="col-md-8 fs-4 mt-4">Controller status:</h2>

          <Row>
            <Col className="col-md-12 col-lg-6">
          <ListGroup horizontal>
            <ListGroup.Item>
                <div className="fw-bold">WiFi Mode</div>
                <div>
                  {status.apMode?"In Access Point mode, functionality is limited. Enter your WiFi credentials below to connect the controller to your home network.":"Connected successfully to the WiFi network! Full controller functionality is available."}
              </div>
            </ListGroup.Item>
            <ListGroup.Item variant={status.apMode?"danger":"success"}>
              <div className="fw-bold" >{status.apMode?"Access Point":"Connected"}</div>
            </ListGroup.Item>
            </ListGroup>
            </Col>
            <Col className="col-md-12 col-lg-6">
            <ListGroup horizontal>
            <ListGroup.Item>
                <div className="fw-bold">Mqtt Status</div>
                <div>{status.mqttConnected?
                  "Mqtt connection established successfully! Home Assistant integration is enabled.":
                  "Mqtt connection error. Enter your Mqtt Server details on the Network Setup page."}</div>
            </ListGroup.Item>
            <ListGroup.Item variant={status.mqttConnected?"success":"danger"} >
              <div className="fw-bold" >{status.mqttConnected?"Connected":"Disconnected"}</div>
            </ListGroup.Item>
            </ListGroup>
          </Col>
          </Row>  
          {status.apMode ? <WifiSetup /> : <BasicLightControls />}

          <p>Copyright ©️ 2025 Mark Godwin.</p>
        </Container>
      </div> }
    </>
  );
}