import { WifiSetup } from './WifiSetup';
import { MqttSetup } from './MqttSetup';
import { FirmwareBoot } from './FirmwareBoot';

export function SetupPage() : JSX.Element {
    return (
      <div>
        <WifiSetup />
        <MqttSetup />
        <FirmwareBoot />
      </div>
    );
  }
