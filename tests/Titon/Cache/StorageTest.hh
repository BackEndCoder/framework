<?hh
namespace Titon\Cache;

use Titon\Cache\Storage\AbstractStorage;
use Titon\Test\TestCase;

/**
 * @property \Titon\Cache\Storage $object
 */
class StorageTest extends TestCase {

    protected function setUp() {
        parent::setUp();

        $this->object = new StorageStub(Map {
            'expires' => '+3 days'
        });
    }

    public function testExpires() {
        $time = time();

        $this->assertEquals(0, $this->object->expires(0));
        $this->assertEquals(strtotime('+3 days'), $this->object->expires(null));
        $this->assertEquals(strtotime('+1 day'), $this->object->expires('+1 day'));
        $this->assertEquals($time, $this->object->expires($time));
        $this->assertEquals(3600, $this->object->expires('+60 minutes', true)); // TTL
    }

    public function testKey() {
        $this->assertEquals('Model-someMethod', $this->object->key('Model::someMethod()'));
        $this->assertEquals('Model-someMethod-123456', $this->object->key('Model::someMethod-123456'));
        $this->assertEquals('Model-someMethod--abc-123456', $this->object->key('Model::someMethod()-abc-123456'));
        $this->assertEquals('some-name-space-Model-someMethod', $this->object->key('some\name\space\Model::someMethod()'));
        $this->assertEquals('cache-key', $this->object->key('cache-key'));
    }

    public function testKeyWithPrefix() {
        $this->object->setConfig('prefix', 'foobar-');

        $this->assertEquals('foobar-cache-key', $this->object->key('cache-key'));
    }

    public function testParseServer() {
        $this->assertEquals(Vector {'server1.com', 'server2.com'}, $this->object->parseServer(['server1.com', 'server2.com']));
        $this->assertEquals(Vector {'server1.com', 'server2.com'}, $this->object->parseServer(Vector {'server1.com', 'server2.com'}));
        $this->assertEquals(Vector {'server1.com', 666, null}, $this->object->parseServer('server1.com', 666));
        $this->assertEquals(Vector {'server1.com', 1337, null}, $this->object->parseServer('server1.com:1337', 666));
        $this->assertEquals(Vector {'server1.com', 666, 10}, $this->object->parseServer('server1.com', 666, 10));
        $this->assertEquals(Vector {'server1.com', 1337, 20}, $this->object->parseServer('server1.com:1337:20', 666, 10));
    }

    public function testStats() {
        $this->assertInstanceOf('HH\Map', $this->object->stats());
    }

}

class StorageStub extends AbstractStorage {

    public function decrement(string $key, int $step = 1): ?int {}
    public function flush(): bool {}
    public function get(string $key): ?mixed {}
    public function has(string $key): bool {}
    public function increment(string $key, int $step = 1): ?int {}
    public function remove(string $key): bool {}
    public function set(string $key, ?mixed $value, mixed $expires = ''): bool {}

}