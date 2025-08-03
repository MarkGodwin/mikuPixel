
import { useCallback, useState } from 'react';
import { Button, Form } from 'react-bootstrap';
import { useNavigate } from 'react-router-dom';

export function AddPatternPage(): JSX.Element {
    const navigate = useNavigate();
    const [name, setName] = useState("");

    const save = useCallback(async () => {
        if (name.trim() === "") {
            alert("Please enter a pattern name.");
            return;
        }
        let params = new URLSearchParams();
        params.append("name", name.trim());
        const response = await fetch(`/api/patterns/add.json?${params.toString()}`);
        if (!response.ok) {
            throw new Error("Failed to save pattern");
        }
        const newId : number = await response.json();

        navigate(`/patterns/edit/${newId}`);
        
    }, [navigate, name]);

    return (
        <div id="AddPattern">
            <h1 className="mt-4">
                Add Pattern
            </h1>
            <Form>
                <Form.Group controlId="formPatternName">
                    <Form.Label>Pattern Name</Form.Label>
                    <Form.Control type="text" placeholder="Enter pattern name" onChange={e => setName(e.target.value)} />
                </Form.Group>
                <Button className="mt-4" variant="submit" onClick={() => save()}>
                    Save Pattern
                </Button>
            </Form>
        </div>
    );
}