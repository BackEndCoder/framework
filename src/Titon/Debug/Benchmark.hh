<?hh // strict
/**
 * @copyright   2010-2013, The Titon Project
 * @license     http://opensource.org/licenses/bsd-license.php
 * @link        http://titon.io
 */

namespace Titon\Debug;

use Titon\Debug\Exception\MissingBenchmarkException;

/**
 * Delivers the functionality to start, stop and log benchmarks.
 * Benchmarks store the time difference and memory usage between two blocks during runtime.
 *
 * @package Titon\Debug
 */
class Benchmark {

    /**
     * User and system initiated benchmarking tests.
     *
     * @type Map<string, Map<string, num>>
     */
    protected static Map<string, Map<string, num>> $_benchmarks = Map {};

    /**
     * Return all benchmarks and calculate averages.
     *
     * @return Map<string, Map<string, num>>
     */
    public static function all(): Map<string, Map<string, num>> {
        $benchmarks = Map {};

        foreach (static::$_benchmarks as $key => $bm) {
            $benchmarks[$key] = static::get($key);
        }

        return $benchmarks;
    }

    /**
     * Return a single benchmark by key and calculate averages and memory usage.
     *
     * @param string $key
     * @return Map<string, num>
     * @throws \Titon\Debug\Exception\MissingBenchmarkException
     */
    public static function get(string $key): Map<string, num> {
        if (!static::$_benchmarks->contains($key)) {
            throw new MissingBenchmarkException(sprintf('Benchmark %s does not exist', $key));
        }

        $bm = static::$_benchmarks[$key];
        $bm['avgTime'] = $bm->contains('endTime') ? ($bm['endTime'] - $bm['startTime']) : 0;
        $bm['avgMemory'] = $bm->contains('endMemory') ? ($bm['endMemory'] - $bm['startMemory']) : 0;
        $bm['peakMemory'] = (float) memory_get_peak_usage();

        return $bm;
    }

    /**
     * Outputs and formats a benchmark directly as a string.
     *
     * @param string $key
     * @return string
     */
    public static function output(string $key): string {
        $benchmark = static::get($key);

        return sprintf('[%s] %s seconds, %s memory (%s peak)',
            $key,
            number_format($benchmark['avgTime'], 4),
            $benchmark['avgMemory'],
            $benchmark['peakMemory']);
    }

    /**
     * Start the benchmarking process by logging the micro seconds and memory usage.
     *
     * @param string $key
     */
    public static function start(string $key): void {
        static::$_benchmarks[$key] = Map {
            'startTime' => microtime(true),
            'startMemory' => memory_get_usage(true),
        };
    }

    /**
     * Stop the benchmarking process by logging the micro seconds and memory usage.
     *
     * @param string $key
     */
    public static function stop(string $key): void {
        static::$_benchmarks[$key]->setAll(Map {
            'endTime' => microtime(true),
            'endMemory' => memory_get_usage(true)
        });
    }

}
