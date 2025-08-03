
import { useEffect, useState } from 'react';
import { ListGroup, Container, Row, Button } from "react-bootstrap";

import { useToaster } from './toaster';
import { Spinner } from "./Spinner";
import { PatternListEntry } from './Pattern';
import { Navigate, useNavigate } from 'react-router-dom';

export function PatternsPage(): JSX.Element {

    const [loading, setLoading] = useState(true);
    const [reload, setReload] = useState(0);
    const [patterns, setPatterns] = useState<PatternListEntry[]>([]);
    const navigate = useNavigate();

    const toaster = useToaster();

    useEffect(() => {
        let cancelled = false;
        let timeout: any;

        async function getPatterns() {

            try {
                const response: PatternListEntry[] = await (await fetch("/api/patterns/list.json")).json();

                if (!cancelled) {
                    setPatterns(response);
                }
            }
            catch(e: any) {
                toaster.open("Failed to read", (<><p>"The list of custom patterns could not be read. Check the device is running."</p><p>{e.toString()}</p></>));
            }
            finally{
                setLoading(false);
            }
        };

        getPatterns();

        return () => {
            cancelled = true;
            clearTimeout(timeout);
        };
    }, [reload, toaster]);



    return (
        <div id="Control">
            <h1 className="mt-4">
                Patterns
            </h1>
            <Spinner loading={loading}></Spinner>
            <ListGroup>
                {patterns.map((pattern, index) => ( <ListGroup.Item key={pattern.id} onClick={() => navigate(`/patterns/edit/${pattern.id}`)}>{pattern.name}</ListGroup.Item> )) }
            </ListGroup>
            <h1 className="mt-4">
                Custom Remotes
            </h1>
            <Button className="mt-4" onClick={() => navigate("/patterns/add")}>Add Pattern</Button>
        </div>
    );
}