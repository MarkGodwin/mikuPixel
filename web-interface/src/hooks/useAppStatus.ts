import { useState, useEffect } from 'react';

interface AppStatus {
    apMode: boolean;
    mqttConnected: boolean;
}

export function useAppStatus(pollInterval = 10000, keepPolling: boolean = false) {
    const [appStatus, setAppStatus] = useState<AppStatus|null>(null);
    const [loadCount, setLoadCount] = useState(0);

    useEffect(() => {
        let cancelled = false;
        let timeout: number | undefined = undefined;

        async function getStatus() {
            var ok = false;
            try {
                const response : AppStatus = await (await fetch("/api/status.json")).json();
                ok = true;
                if(!cancelled) {
                    setAppStatus(response);
                }
            }
            catch(ex) {
                console.log(ex);
            }
            // Cause the status info to reload
            if(!cancelled && ok && keepPolling)
            {
                timeout = window.setTimeout(() => {
                    setLoadCount(n => n+1);
                }, pollInterval);
            }
        };

        getStatus();
        return () => {
            cancelled = true;
            window.clearTimeout(timeout);
        };
    }, [loadCount, pollInterval, keepPolling]);

    return appStatus;
}
