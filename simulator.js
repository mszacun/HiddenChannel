function HiddenMessageQueue($el) {
    this.$el = $el;
    this.padding = 2;
    this.messages = [];
    this.addAnimationLenght = 1000;
    this.removeAnimationLength = 1000;
}

HiddenMessageQueue.prototype.addToQueue = function(data, completeCallback) {
    $newMessage = $('<div>', {class: 'hidden-message'}).text(data);
    $newMessage.css('left', 200)

    var animationStopX = this.padding;

    for (var i = 0; i < this.messages.length; i++)
        animationStopX += $(this.messages[i]).width() + this.padding;

    this.$el.append($newMessage);
    this.messages.push($newMessage);

    $newMessage.animate({left: animationStopX}, this.addAnimationLenght, completeCallback);
}

HiddenMessageQueue.prototype.addOnTheRight = function(data, completeCallback) {
    $newMessage = $('<div>', {class: 'hidden-message'}).text(data);
    $newMessage.css('right', 200)

    var animationStopX = this.padding;

    for (var i = 0; i < this.messages.length; i++)
        animationStopX += $(this.messages[i]).width() + this.padding;

    this.$el.append($newMessage);
    this.messages.push($newMessage);

    $newMessage.animate({right: animationStopX}, this.addAnimationLenght, completeCallback);
}

HiddenMessageQueue.prototype.shift = function(completeCallback) {
    var $removedMessage = $(this.messages.shift());

    $removedMessage.animate({top: '+=40'}, this.removeAnimationLength);
    for (var i = 0; i < this.messages.length; i++)
        $(this.messages[i]).animate({left: '-=' + $removedMessage.width()}, this.removeAnimationLength);

    return $removedMessage;
}

function Queue($el) {
    this.$el = $el
    this.packets = [];
    this.numberOfItems = 0;
    this.padding = 5;
    this.horizontalMargin = 10;
    this.addRemoveAnimationTime = 1000;
}

Queue.prototype.addMessageOnTop = function(size, completeCallback) {
    this.numberOfItems++;
    var animationStopY = this.padding;

    for (var i = 0; i < this.packets.length; i++) {
        var pac = $(this.packets[i]);
        var w = pac.height();
        animationStopY += w + this.padding;
    }

    var $newPacket = $('<div>', {class: 'message'});
    var $queue = this.$el

    $queue.append($newPacket);
    this.packets.push($newPacket);
    $newPacket.css('left', this.horizontalMargin);
    $newPacket.css('height', size);

    $newPacket.animate({bottom: animationStopY}, this.addRemoveAnimationTime, completeCallback);
}

Queue.prototype.addMessageOnBottom = function(size, completeCallback) {
    this.numberOfItems++;
    var packets = this.$el.find('.message');

    var $newPacket = $('<div>', {class: 'message'});
    var $queue = this.$el;

    $queue.append($newPacket);
    this.packets.unshift($newPacket);
    $newPacket.css('left', this.horizontalMargin);
    $newPacket.css('bottom', this.padding);
    $newPacket.css('height', 0);

    $newPacket.animate({height: size}, this.addRemoveAnimationTime, completeCallback);
    for (var i = this.numberOfItems - 1; i >= 0; i--)
        $(packets[i]).animate({bottom: '+=' + (size + this.padding)}, this.addRemoveAnimationTime);

}

Queue.prototype.removeBottomPacket = function() {
    var packets = this.$el.find('.message');
    var $packetToRemove = $(packets[0]);
    var heightToShiftOtherPackets = $packetToRemove.height();

    $packetToRemove.animate({height: 0});
    for (var i = 1; i < this.numberOfItems; i++)
        $(packets[i]).animate({bottom: '-=' + heightToShiftOtherPackets});

}

function fillQueueWithPackets(queue, packetsLength, i) {
    if (i < packetsLength.length)
        queue.addMessageOnTop(packetsLength[i], function () {
            fillQueueWithPackets(queue, packetsLength, i + 1);
        });
}

function Link($el) {
    this.$el = $el;
}

Link.prototype.sendPacket = function () {
    var $packet = $('<div>', {class: 'message', height: 40});
    var $header = $('<div>', {class: 'header', height: 40});
    var $desc = $('<div>', {class: 'packet-description'}).text('Rozmiar: 97 bajtow [a]');
    var $arrow = $('<i>', {class: 'fa fa-arrow-right packet-arrow'})
    var $together = $('<div>', {class: 'packet'}).append($desc).append($header).append($packet).append($arrow);
    this.$el.append($together);

    $together.animate({left: '+=350'}, 2000);
}
