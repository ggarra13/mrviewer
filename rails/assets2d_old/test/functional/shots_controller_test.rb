require File.dirname(__FILE__) + '/../test_helper'
require 'shots_controller'

# Re-raise errors caught by the controller.
class ShotsController; def rescue_action(e) raise e end; end

class ShotsControllerTest < Test::Unit::TestCase
  fixtures :shots

  def setup
    @controller = ShotsController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new

    @first_id = shots(:first).id
  end

  def test_index
    get :index
    assert_response :success
    assert_template 'list'
  end

  def test_list
    get :list

    assert_response :success
    assert_template 'list'

    assert_not_nil assigns(:shots)
  end

  def test_show
    get :show, :id => @first_id

    assert_response :success
    assert_template 'show'

    assert_not_nil assigns(:shot)
    assert assigns(:shot).valid?
  end

  def test_new
    get :new

    assert_response :success
    assert_template 'new'

    assert_not_nil assigns(:shot)
  end

  def test_create
    num_shots = Shot.count

    post :create, :shot => {}

    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_equal num_shots + 1, Shot.count
  end

  def test_edit
    get :edit, :id => @first_id

    assert_response :success
    assert_template 'edit'

    assert_not_nil assigns(:shot)
    assert assigns(:shot).valid?
  end

  def test_update
    post :update, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'show', :id => @first_id
  end

  def test_destroy
    assert_nothing_raised {
      Shot.find(@first_id)
    }

    post :destroy, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_raise(ActiveRecord::RecordNotFound) {
      Shot.find(@first_id)
    }
  end
end
