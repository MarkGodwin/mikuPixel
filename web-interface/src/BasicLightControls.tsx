import { useCallback, useRef, useState } from 'react';
import { Card, Form } from 'react-bootstrap';
import { useToaster } from './toaster';
import { setBrightness, setRgb } from './api';


export function BasicLightControls() {
    const [brightness, setBrightnessLocal] = useState(100);
    const [colour, setColourLocal] = useState('#ffffff');
    const toaster = useToaster();
    
    const timeoutRef = useRef<ReturnType<typeof setTimeout> | null>(null);

    const handleColourChange = useCallback(async (hexColor: string) =>
        {
            setColourLocal(hexColor);

        if (timeoutRef.current) {
            clearTimeout(timeoutRef.current);
        }

        timeoutRef.current = setTimeout(async () => {
            try {
                // Convert hex to RGB
                const r = parseInt(hexColor.slice(1, 3), 16);
                const g = parseInt(hexColor.slice(3, 5), 16);
                const b = parseInt(hexColor.slice(5, 7), 16);
                await setRgb(r, g, b);
            } catch (error) {
                toaster.open('Error', 'Failed to update color. Check device connection.');
            }
        }, 300);
    }, [toaster]);

    // Stable API update functions
    // Update handlers
    const handleBrightnessChange = useCallback(async (value: number) => {
        setBrightnessLocal(value);

        if (timeoutRef.current) {
            clearTimeout(timeoutRef.current);
        }

        timeoutRef.current = setTimeout(async () => {
            try {
                await setBrightness(value);
            } catch (error) {
                toaster.open('Error', 'Failed to update brightness. Check device connection.');
            }
        }, 300);
    }, [toaster]);

    return (
        <Card className="mt-4">
            <Card.Header>
                <h3 className="mb-0">Basic Light Controls</h3>
            </Card.Header>
            <Card.Body>
                <Form>
                    <Form.Group className="mb-3">
                        <Form.Label>Brightness</Form.Label>
                        <Form.Range 
                            value={brightness}
                            onChange={(e) => handleBrightnessChange(parseInt(e.target.value, 10))}
                            min={0}
                            max={255}
                        />
                        <div className="text-muted text-end">{brightness}%</div>
                    </Form.Group>

                    <Form.Group>
                        <Form.Label>Color</Form.Label>
                        <Form.Control
                            type="color"
                            value={colour}
                            onChange={(e) => handleColourChange(e.target.value)}
                            title="Choose color"
                        />
                    </Form.Group>
                </Form>
            </Card.Body>
        </Card>
    );
}
