
SELECT plan(4);

SELECT has_function('pgr_bellman_ford');
SELECT can(ARRAY['pgr_bellman_ford']);

SELECT has_function('pgr_bellman_ford', ARRAY['text','bigint','bigint','boolean', 'boolean']);
SELECT has_function('pgr_bellman_ford', ARRAY['text','bigint','anyarray','boolean', 'boolean']);
SELECT has_function('pgr_bellman_ford', ARRAY['text','anyarray','bigint','boolean', 'boolean']);
SELECT has_function('pgr_bellman_ford', ARRAY['text','anyarray','anyarray','boolean', 'boolean']);

SELECT function_returns('pgr_bellman_ford', ARRAY['text','bigint','bigint','boolean', 'boolean'],'setof record');
SELECT function_returns('pgr_bellman_ford', ARRAY['text','bigint','anyarray','boolean', 'boolean'],'setof record');
SELECT function_returns('pgr_bellman_ford', ARRAY['text','anyarray','bigint','boolean', 'boolean'],'setof record');
SELECT function_returns('pgr_bellman_ford', ARRAY['text','anyarray','anyarray','boolean', 'boolean'],'setof record');

-- testing column names
SELECT bag_has(
    $$SELECT  proargnames from pg_proc where proname = 'pgr_bellman_ford'$$,
    $$SELECT  '{"","","","directed","only_cost","seq","path_seq","node","edge","cost","agg_cost"}'::TEXT[] $$
);

